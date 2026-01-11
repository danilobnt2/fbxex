import React from "react";
import { describe, expect, it } from "vitest";
import { render, screen } from "@testing-library/react";
import { Provider } from "react-redux";
import { AnyAction, configureStore } from "@reduxjs/toolkit";

import HierarchyTreeView, { IHierarchyTreeService } from "./hierarchytreeview";
import { TreeNode, TreeNodeId } from "../../viewmodels/treeview";

type TreeState = {
  tree?: {
    rootId: TreeNodeId;
    nodes: Record<TreeNodeId, TreeNode>;
    selectedNodeId: TreeNodeId | null;
  };
};

const renderWithState = (ui: React.ReactElement, preloadedState: TreeState, service: IHierarchyTreeService) => {
  const actions: AnyAction[] = [];
  const store = configureStore({
    reducer: (state = preloadedState, action) => {
      actions.push(action);
      return state;
    },
    preloadedState,
  });

  const result = render(<Provider store={store}>{ui}</Provider>);
  return { ...result, actions };
};

const createService = (overrides: Partial<IHierarchyTreeService> = {}): IHierarchyTreeService => ({
  getNode: (state, id) => (state as TreeState).tree?.nodes?.[id],
  createCollapseNodeAction: (id) => ({ type: "TREE/COLLAPSED", payload: { id } }),
  createRequestExpandNodeAction: (id) => ({ type: "TREE/EXPAND_REQUESTED", payload: { id } }),
  createSelectNodeAction: (id) => ({ type: "TREE/NODE_SELECTED", payload: { id } }),
  ...overrides,
});

const makeNode = (overrides: Partial<TreeNode>): TreeNode => ({
  id: 0,
  parentId: null,
  children: null,
  previewProperties: { name: "Node" },
  isExpanded: false,
  isLoading: false,
  isSelected: false,
  ...overrides,
});

describe("HierarchyTreeView", () => {
  it("returns null when the root node is missing", () => {
    const service = createService();
    const { container } = renderWithState(<HierarchyTreeView {...service} />, {}, service);

    expect(container.firstChild).toBeNull();
  });

  it("shows the root node and empty prompt when no children are loaded", () => {
    const service = createService();
    const state: TreeState = {
      tree: {
        rootId: 0,
        selectedNodeId: null,
        nodes: {
          0: makeNode({ previewProperties: { name: "Root" }, children: null, isExpanded: true }),
        },
      },
    };

    renderWithState(<HierarchyTreeView {...service} />, state, service);

    expect(screen.getByText(/Root/)).toBeInTheDocument();
    expect(screen.getByText("#0")).toBeInTheDocument();
    expect(screen.getByText("Open an FBX file to populate the tree.")).toBeInTheDocument();
  });

  it("dispatches collapse when toggling an expanded node", () => {
    const service = createService();
    const state: TreeState = {
      tree: {
        rootId: 0,
        selectedNodeId: null,
        nodes: {
          0: makeNode({ id: 0, isExpanded: true, children: [1] }),
          1: makeNode({ id: 1, parentId: 0, children: [] }),
        },
      },
    };

    const { actions } = renderWithState(<HierarchyTreeView {...service} />, state, service);

    screen.getByLabelText("Collapse node").click();

    expect(actions.some((a) => a.type === "TREE/COLLAPSED" && a.payload.id === 0)).toBe(true);
  });

  it("dispatches expand when toggling a collapsed node", () => {
    const service = createService();
    const state: TreeState = {
      tree: {
        rootId: 0,
        selectedNodeId: null,
        nodes: {
          0: makeNode({ id: 0, isExpanded: false, children: [1] }),
          1: makeNode({ id: 1, parentId: 0, children: [] }),
        },
      },
    };

    const { actions } = renderWithState(<HierarchyTreeView {...service} />, state, service);

    screen.getByLabelText("Expand node").click();

    expect(actions.some((a) => a.type === "TREE/EXPAND_REQUESTED" && a.payload.id === 0)).toBe(true);
  });

  it("dispatches select when a node label is clicked", () => {
    const service = createService();
    const state: TreeState = {
      tree: {
        rootId: 0,
        selectedNodeId: null,
        nodes: {
          0: makeNode({ id: 0, previewProperties: { name: "Root" }, children: [1], isExpanded: true }),
          1: makeNode({ id: 1, parentId: 0, previewProperties: { name: "Child" }, children: [] }),
        },
      },
    };

    const { actions } = renderWithState(<HierarchyTreeView {...service} />, state, service);

    screen.getByText(/Child/).click();

    expect(actions.some((a) => a.type === "TREE/NODE_SELECTED" && a.payload.id === 1)).toBe(true);
  });

  it("shows loading, error, and leaf indicators", () => {
    const service = createService();
    const state: TreeState = {
      tree: {
        rootId: 0,
        selectedNodeId: null,
        nodes: {
          0: makeNode({ id: 0, isExpanded: true, isLoading: true, children: [1, 2] }),
          1: makeNode({ id: 1, parentId: 0, error: "Failed to load", children: [] }),
          2: makeNode({ id: 2, parentId: 0, children: [] }),
        },
      },
    };

    renderWithState(<HierarchyTreeView {...service} />, state, service);

    expect(screen.getAllByText("Loading")[0]).toBeInTheDocument();
    expect(screen.getByText("Failed to load")).toBeInTheDocument();
    expect(screen.getAllByText("No children")[0]).toBeInTheDocument();
  });
});
