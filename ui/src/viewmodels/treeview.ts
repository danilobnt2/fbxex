import { AnyAction, Middleware, UnknownAction } from "@reduxjs/toolkit";
import ul from "../ul";

export type TreeNodeId = number;

export type TreeNode = {
  id: TreeNodeId;
  parentId: TreeNodeId | null;
  children: TreeNodeId[] | null;
  isExpanded: boolean;
  isLoading: boolean;
  error?: string;
};

export type TreeViewState = {
  rootId: TreeNodeId;
  nodes: Record<TreeNodeId, TreeNode>;
};

const createNode = (id: TreeNodeId, parentId: TreeNodeId | null = null): TreeNode => ({
  id,
  parentId,
  children: null,
  isExpanded: false,
  isLoading: false,
});

export const initialTreeViewState: TreeViewState = {
  rootId: 0,
  nodes: { 0: createNode(0, null) },
};

export type TreeViewAction =
  | { type: "TREE/RESET"; payload?: { rootId?: TreeNodeId } }
  | { type: "TREE/EXPAND_REQUESTED"; payload: { id: TreeNodeId } }
  | { type: "TREE/COLLAPSED"; payload: { id: TreeNodeId } }
  | { type: "TREE/CHILDREN_LOADED"; payload: { id: TreeNodeId; children: TreeNodeId[] } }
  | { type: "TREE/EXPAND_FAILED"; payload: { id: TreeNodeId; error: string } };

export const resetTree = (rootId: TreeNodeId = 0): TreeViewAction => ({
  type: "TREE/RESET",
  payload: { rootId },
});

export const requestExpandNode = (id: TreeNodeId): TreeViewAction => ({
  type: "TREE/EXPAND_REQUESTED",
  payload: { id },
});

export const collapseNode = (id: TreeNodeId): TreeViewAction => ({
  type: "TREE/COLLAPSED",
  payload: { id },
});

export const treeChildrenLoaded = (id: TreeNodeId, children: TreeNodeId[]): TreeViewAction => ({
  type: "TREE/CHILDREN_LOADED",
  payload: { id, children },
});

export const treeExpandFailed = (id: TreeNodeId, error: string): TreeViewAction => ({
  type: "TREE/EXPAND_FAILED",
  payload: { id, error },
});

export const treeViewReducer = (
  state: TreeViewState = initialTreeViewState,
  action: TreeViewAction | AnyAction
): TreeViewState => {
  switch (action.type) {
    case "TREE/RESET": {
      const rootId = action.payload?.rootId ?? 0;
      return {
        rootId,
        nodes: { [rootId]: createNode(rootId, null) },
      };
    }
    case "TREE/EXPAND_REQUESTED": {
      const node = state.nodes[action.payload.id] ?? createNode(action.payload.id, null);
      const shouldLoad = node.children === null;
      return {
        ...state,
        nodes: {
          ...state.nodes,
          [action.payload.id]: {
            ...node,
            isExpanded: node.children !== null ? true : node.isExpanded,
            isLoading: shouldLoad,
            error: undefined,
          },
        },
      };
    }
    case "TREE/CHILDREN_LOADED": {
      const { id, children } = action.payload;
      const parent = state.nodes[id] ?? createNode(id, null);
      const nodes = { ...state.nodes };

      nodes[id] = {
        ...parent,
        children,
        isExpanded: true,
        isLoading: false,
        error: undefined,
      };

      children.forEach((childId) => {
        const child = nodes[childId];
        nodes[childId] = child ? { ...child, parentId: child.parentId ?? id } : createNode(childId, id);
      });

      return { ...state, nodes };
    }
    case "TREE/EXPAND_FAILED": {
      const node = state.nodes[action.payload.id] ?? createNode(action.payload.id, null);
      return {
        ...state,
        nodes: {
          ...state.nodes,
          [action.payload.id]: {
            ...node,
            isLoading: false,
            error: action.payload.error,
          },
        },
      };
    }
    case "TREE/COLLAPSED": {
      const node = state.nodes[action.payload.id] ?? createNode(action.payload.id, null);
      return {
        ...state,
        nodes: {
          ...state.nodes,
          [action.payload.id]: { ...node, isExpanded: false },
        },
      };
    }
    default:
      return state;
  }
};

export type TreeAwareState = { tree: TreeViewState };

export const treeViewMiddleware: Middleware<{}, TreeAwareState> = (storeApi) => (next) => (action: UnknownAction) => {
  if (action.type !== "TREE/EXPAND_REQUESTED") {
    return next(action);
  }

  const { id } = action.payload as { id: TreeNodeId };
  const currentNode = storeApi.getState().tree.nodes[id];

  if (currentNode && (currentNode.children !== null || currentNode.isLoading)) {
    return next(action);
  }

  const result = next(action);

  try {
    if (!ul.isAvailable) {
      throw new Error("Ultralight bridge is not available.");
    }
    const children = ul.getFBXNodeChildren(id);
    storeApi.dispatch(treeChildrenLoaded(id, children));
  } catch (err) {
    const message = err instanceof Error ? err.message : String(err);
    storeApi.dispatch(treeExpandFailed(id, message));
  }

  return result;
};
