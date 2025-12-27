import { configureStore } from "@reduxjs/toolkit";
import { describe, it, expect, vi, beforeEach } from "vitest";

import {
  TreeViewState,
  createTreeViewMiddleware,
  initialTreeViewState,
  requestExpandNode,
  collapseNode,
  resetTree,
  selectNode,
  selectTreeNode,
  treeChildrenLoaded,
  treePropertiesFailed,
  treeViewReducer,
  ITreePlatform,
} from "./treeview";

const createMockPlatform = (overrides: Partial<ITreePlatform> = {}): ITreePlatform => ({
  isPlatformAvailable: vi.fn(() => true),
  getFBXNodeChildren: vi.fn(() => []),
  getFBXNodeProperties: vi.fn(() => ({})),
  getFBXPreviewProperties: vi.fn(() => ({})),
  ...overrides,
});

const createTreeStore = (platformOverrides: Partial<ITreePlatform> = {}) => {
  const platform = createMockPlatform(platformOverrides);
  const store = configureStore({
    reducer: { tree: treeViewReducer },
    middleware: (getDefaultMiddleware) => getDefaultMiddleware().concat(createTreeViewMiddleware(platform)),
  });
  return { store, platform };
};

describe("treeViewReducer", () => {
  beforeEach(() => {
    vi.clearAllMocks();
  });

  it("resets tree state with provided root id", () => {
    const populatedState: TreeViewState = {
      ...initialTreeViewState,
      selectedNodeId: 4,
      nodes: {
        ...initialTreeViewState.nodes,
        4: { ...initialTreeViewState.nodes[0], id: 4, isSelected: true },
      },
    };

    const nextState = treeViewReducer(populatedState, resetTree(5));

    expect(nextState.rootId).toBe(5);
    expect(Object.keys(nextState.nodes)).toEqual(["5"]);
    expect(nextState.nodes[5].parentId).toBeNull();
    expect(nextState.selectedNodeId).toBeNull();
  });

  it("marks a node as loading when expanding without known children", () => {
    const nextState = treeViewReducer(undefined, requestExpandNode(0));

    expect(nextState.nodes[0].isLoading).toBe(true);
    expect(nextState.nodes[0].isExpanded).toBe(false);
    expect(nextState.nodes[0].error).toBeUndefined();
  });

  it("marks an existing node as expanded when children already exist", () => {
    const withChildren = treeViewReducer(undefined, treeChildrenLoaded(0, [{ id: 2 }]));
    const expanded = treeViewReducer(withChildren, requestExpandNode(0));

    expect(expanded.nodes[0].isExpanded).toBe(true);
    expect(expanded.nodes[0].isLoading).toBe(false);
  });

  it("collapses a node", () => {
    const expanded = treeViewReducer(undefined, treeChildrenLoaded(0, [{ id: 2 }]));
    const collapsed = treeViewReducer(expanded, collapseNode(0));

    expect(collapsed.nodes[0].isExpanded).toBe(false);
  });

  it("handles expand failure on unknown node", () => {
    const failed = treeViewReducer(undefined, { type: "TREE/EXPAND_FAILED", payload: { id: 4, error: "oops" } });

    expect(failed.nodes[4].error).toBe("oops");
    expect(failed.nodes[4].isLoading).toBe(false);
  });

  it("tracks existing children and preserves them on expand", () => {
    const withChildren = treeViewReducer(undefined, treeChildrenLoaded(0, [{ id: 1 }], { name: "Root" }));
    const expanded = treeViewReducer(withChildren, requestExpandNode(0));

    expect(expanded.nodes[0].isExpanded).toBe(true);
    expect(expanded.nodes[0].isLoading).toBe(false);
    expect(expanded.nodes[0].children).toEqual([1]);
  });

  it("stores loaded children and preview properties", () => {
    const nextState = treeViewReducer(
      undefined,
      treeChildrenLoaded(
        0,
        [
          { id: 10, previewProperties: { name: "Child 10" } },
          { id: 11 },
        ],
        { name: "Custom root" }
      )
    );

    expect(nextState.nodes[0].previewProperties.name).toBe("Custom root");
    expect(nextState.nodes[0].children).toEqual([10, 11]);
    expect(nextState.nodes[10].parentId).toBe(0);
    expect(nextState.nodes[10].previewProperties.name).toBe("Child 10");
    expect(nextState.nodes[11].previewProperties.name).toContain(".$Node 11");
  });

  it("updates existing child nodes when children are reloaded", () => {
    const preloadedState: TreeViewState = {
      ...initialTreeViewState,
      nodes: {
        ...initialTreeViewState.nodes,
        5: { ...initialTreeViewState.nodes[0], id: 5, parentId: null, previewProperties: { name: "Old name" } },
      },
    };

    const nextState = treeViewReducer(preloadedState, treeChildrenLoaded(0, [{ id: 5 }]));

    expect(nextState.nodes[5].parentId).toBe(0);
    expect(nextState.nodes[5].previewProperties.name).toBe("Old name");
  });

  it("selects a node and clears any previous selection", () => {
    const withChildren = treeViewReducer(undefined, treeChildrenLoaded(0, [{ id: 1 }]));
    const selected = treeViewReducer(withChildren, selectNode(1));

    expect(selected.nodes[0].isSelected).toBe(false);
    expect(selected.nodes[1].isSelected).toBe(true);
    expect(selected.selectedNodeId).toBe(1);
    expect(selected.nodes[1].error).toBeUndefined();
  });

  it("records property load failure even for unknown nodes", () => {
    const nextState = treeViewReducer(undefined, treePropertiesFailed(9, "Cannot read properties"));

    expect(nextState.nodes[9]).toBeDefined();
    expect(nextState.nodes[9].error).toBe("Cannot read properties");
  });

  it("stores properties and clears previous errors", () => {
    const errored = treeViewReducer(undefined, treePropertiesFailed(0, "bad"));
    const loaded = treeViewReducer(errored, { type: "TREE/PROPERTIES_LOADED", payload: { id: 0, properties: { foo: 1 } } });

    expect(loaded.nodes[0].properties).toEqual({ foo: 1 });
    expect(loaded.nodes[0].error).toBeUndefined();
  });

  it("selectTreeNode returns same action shape as selectNode", () => {
    expect(selectTreeNode(3)).toEqual(selectNode(3));
  });
});

describe("createTreeViewMiddleware", () => {
  beforeEach(() => {
    vi.clearAllMocks();
  });

  it("fetches children when expanding a node without cached data", () => {
    const { store, platform } = createTreeStore({
      getFBXNodeChildren: vi.fn(() => [2, 3]),
      getFBXPreviewProperties: vi.fn((id: number) => ({ name: `Node ${id}` })),
    });

    store.dispatch(requestExpandNode(0));

    expect(platform.isPlatformAvailable).toHaveBeenCalledTimes(1);
    expect(platform.getFBXNodeChildren).toHaveBeenCalledWith(0);
    expect(platform.getFBXPreviewProperties).toHaveBeenCalledWith(0);
    expect(store.getState().tree.nodes[0].children).toEqual([2, 3]);
    expect(store.getState().tree.nodes[2].parentId).toBe(0);
    expect(store.getState().tree.nodes[2].previewProperties.name).toBe("Node 2");
    expect(store.getState().tree.nodes[0].isLoading).toBe(false);
  });

  it("skips fetch when children are already loaded", () => {
    const { store, platform } = createTreeStore({
      getFBXNodeChildren: vi.fn(() => [5]),
    });

    store.dispatch(treeChildrenLoaded(0, [{ id: 5 }]));
    platform.getFBXNodeChildren.mockClear();

    store.dispatch(requestExpandNode(0));

    expect(platform.getFBXNodeChildren).not.toHaveBeenCalled();
    expect(platform.isPlatformAvailable).not.toHaveBeenCalled();
    expect(store.getState().tree.nodes[0].isExpanded).toBe(true);
  });

  it("skips fetch when node expansion is already in-flight", () => {
    const { store, platform } = createTreeStore({
      getFBXNodeChildren: vi.fn(() => [1]),
    });

    store.dispatch(requestExpandNode(0)); // sets isLoading true
    platform.getFBXNodeChildren.mockClear();

    store.dispatch(requestExpandNode(0));

    expect(platform.getFBXNodeChildren).not.toHaveBeenCalled();
    expect(store.getState().tree.nodes[0].isLoading).toBe(false);
  });

  it("dispatches expand failure when platform is unavailable", () => {
    const { store, platform } = createTreeStore({
      isPlatformAvailable: vi.fn(() => false),
    });

    store.dispatch(requestExpandNode(0));

    expect(platform.isPlatformAvailable).toHaveBeenCalledTimes(1);
    expect(store.getState().tree.nodes[0].error).toBe("Ultralight bridge is not available.");
    expect(store.getState().tree.nodes[0].isLoading).toBe(false);
  });

  it("dispatches expand failure when platform throws", () => {
    const { store, platform } = createTreeStore({
      getFBXNodeChildren: vi.fn(() => {
        throw new Error("boom");
      }),
    });

    store.dispatch(requestExpandNode(0));

    expect(platform.getFBXNodeChildren).toHaveBeenCalledWith(0);
    expect(store.getState().tree.nodes[0].error).toBe("boom");
    expect(store.getState().tree.nodes[0].isLoading).toBe(false);
  });

  it("loads properties after selecting a node", () => {
    const properties = { foo: "bar" };
    const { store, platform } = createTreeStore({
      getFBXNodeProperties: vi.fn(() => properties),
    });

    store.dispatch(selectNode(0));

    expect(platform.getFBXNodeProperties).toHaveBeenCalledWith(0);
    expect(store.getState().tree.nodes[0].properties).toEqual(properties);
  });

  it("records property load errors when platform cannot provide data", () => {
    const { store, platform } = createTreeStore({
      isPlatformAvailable: vi.fn(() => false),
    });

    store.dispatch(selectNode(7));

    expect(platform.isPlatformAvailable).toHaveBeenCalledTimes(1);
    expect(store.getState().tree.nodes[7].error).toBe("Ultralight bridge is not available.");
  });
});
