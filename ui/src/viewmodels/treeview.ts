import { AnyAction, Middleware, UnknownAction } from "@reduxjs/toolkit";
import { FBXNodeProps } from "../models/fbxnode";

export type TreeNodeId = number;

export type TreeNode = {
  id: TreeNodeId;
  parentId: TreeNodeId | null;
  children: TreeNodeId[] | null;
  previewProperties: { name: string; attributeTypes?: string[] };
  isExpanded: boolean;
  isLoading: boolean;
  isSelected: boolean;
  properties?: FBXNodeProps;
  error?: string;
};

export type TreeViewState = {
  rootId: TreeNodeId;
  nodes: Record<TreeNodeId, TreeNode>;
  selectedNodeId: TreeNodeId | null;
};

const getDefaultNodeName = (id: TreeNodeId) => `.$Node ${id}`;

const createNode = (
  id: TreeNodeId,
  parentId: TreeNodeId | null = null,
  name?: string,
  attributeTypes?: string[]
): TreeNode => ({
  id,
  parentId,
  previewProperties: { name: name ?? getDefaultNodeName(id), attributeTypes },
  children: null,
  isExpanded: false,
  isLoading: false,
  isSelected: false,
});

export const initialTreeViewState: TreeViewState = {
  rootId: 0,
  nodes: { 0: createNode(0, null) },
  selectedNodeId: null,
};

type TreeChildNode = { id: TreeNodeId; previewProperties?: { name?: string; attributeTypes?: string[] } };

export type TreeViewAction =
  | { type: "TREE/RESET"; payload?: { rootId?: TreeNodeId } }
  | { type: "TREE/EXPAND_REQUESTED"; payload: { id: TreeNodeId } }
  | { type: "TREE/COLLAPSED"; payload: { id: TreeNodeId } }
  | {
      type: "TREE/CHILDREN_LOADED";
      payload: { id: TreeNodeId; previewProperties?: { name?: string; attributeTypes?: string[] }; children: TreeChildNode[] };
    }
  | { type: "TREE/EXPAND_FAILED"; payload: { id: TreeNodeId; error: string } }
  | { type: "TREE/NODE_SELECTED"; payload: { id: TreeNodeId } }
  | { type: "TREE/PROPERTIES_LOADED"; payload: { id: TreeNodeId; properties: FBXNodeProps } }
  | { type: "TREE/PROPERTIES_FAILED"; payload: { id: TreeNodeId; error: string } };

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

export const selectNode = (id: TreeNodeId): TreeViewAction => ({
  type: "TREE/NODE_SELECTED",
  payload: { id },
});

export const treeChildrenLoaded = (
  id: TreeNodeId,
  children: TreeChildNode[],
  previewProperties?: { name?: string; attributeTypes?: string[] }
): TreeViewAction => ({
  type: "TREE/CHILDREN_LOADED",
  payload: { id, children, previewProperties },
});

export const treeExpandFailed = (id: TreeNodeId, error: string): TreeViewAction => ({
  type: "TREE/EXPAND_FAILED",
  payload: { id, error },
});

export const selectTreeNode = (id: TreeNodeId): TreeViewAction => ({
  type: "TREE/NODE_SELECTED",
  payload: { id },
});

export const treePropertiesLoaded = (id: TreeNodeId, properties: FBXNodeProps): TreeViewAction => ({
  type: "TREE/PROPERTIES_LOADED",
  payload: { id, properties },
});

export const treePropertiesFailed = (id: TreeNodeId, error: string): TreeViewAction => ({
  type: "TREE/PROPERTIES_FAILED",
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
        nodes: { [rootId]: createNode(rootId, null), },
        selectedNodeId: null,
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
      const { id, children, previewProperties } = action.payload;
      const parent = state.nodes[id] ?? createNode(id, null, previewProperties?.name, previewProperties?.attributeTypes);
      const nodes = { ...state.nodes };

      nodes[id] = {
        ...parent,
        previewProperties: {
          name: previewProperties?.name ?? parent.previewProperties.name ?? getDefaultNodeName(id),
          attributeTypes: previewProperties?.attributeTypes ?? parent.previewProperties.attributeTypes,
        },
        children: children.map((child) => child.id),
        isExpanded: true,
        isLoading: false,
        error: undefined,
      };

      children.forEach((child) => {
        const existingChild = nodes[child.id];
        const childName =
          child.previewProperties?.name ?? existingChild?.previewProperties?.name ?? getDefaultNodeName(child.id);
        const childAttributeTypes = child.previewProperties?.attributeTypes ?? existingChild?.previewProperties?.attributeTypes;
        nodes[child.id] = existingChild
          ? {
              ...existingChild,
              parentId: existingChild.parentId ?? id,
              previewProperties: { name: childName, attributeTypes: childAttributeTypes },
            }
          : createNode(child.id, id, childName, childAttributeTypes);
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
    case "TREE/NODE_SELECTED": {
      const nodes = Object.entries(state.nodes).reduce<Record<TreeNodeId, TreeNode>>((acc, [key, node]) => {
        const nodeId = Number(key) as TreeNodeId;
        acc[nodeId] = { ...node, isSelected: nodeId === action.payload.id };
        return acc;
      }, {});
      if (!nodes[action.payload.id]) {
        nodes[action.payload.id] = createNode(action.payload.id, null);
      } else {
        nodes[action.payload.id] = { ...nodes[action.payload.id], isSelected: true, error: undefined };
      }
      return { ...state, nodes, selectedNodeId: action.payload.id };
    }
    case "TREE/PROPERTIES_LOADED": {
      const node = state.nodes[action.payload.id] ?? createNode(action.payload.id, null);
      return {
        ...state,
        nodes: {
          ...state.nodes,
          [action.payload.id]: { ...node, properties: action.payload.properties, error: undefined },
        },
      };
    }
    case "TREE/PROPERTIES_FAILED": {
      const node = state.nodes[action.payload.id] ?? createNode(action.payload.id, null);
      return {
        ...state,
        nodes: {
          ...state.nodes,
          [action.payload.id]: { ...node, error: action.payload.error },
        },
      };
    }
    default:
      return state;
  }
};

export type TreeAwareState = { tree: TreeViewState };

export interface ITreePlatform {
  isPlatformAvailable: () => boolean;
  getFBXNodeChildren: (id: TreeNodeId) => TreeNodeId[];
  getFBXNodeProperties: (id: TreeNodeId) => FBXNodeProps;
  getFBXPreviewProperties: (id: TreeNodeId) => { name?: string; attributeTypes?: string[] };
}

export const createTreeViewMiddleware =
  (platform: ITreePlatform): Middleware<{}, TreeAwareState> =>
  (storeApi) =>
  (next) =>
  (action: UnknownAction) => {
    if (action.type === "TREE/EXPAND_REQUESTED") {
      const { id } = action.payload as { id: TreeNodeId };
      const currentNode = storeApi.getState().tree.nodes[id];

      if (currentNode && (currentNode.children !== null || currentNode.isLoading)) {
        return next(action);
      }

      const result = next(action);

      try {
        if (!platform.isPlatformAvailable()) {
          throw new Error("Ultralight bridge is not available.");
        }
        const previewProperties = platform.getFBXPreviewProperties(id);
        const children = platform.getFBXNodeChildren(id).map<TreeChildNode>((childId) => ({
          id: childId,
          previewProperties: platform.getFBXPreviewProperties(childId),
        }));
        storeApi.dispatch(treeChildrenLoaded(id, children, previewProperties));
      } catch (err) {
        const message = err instanceof Error ? err.message : String(err);
        storeApi.dispatch(treeExpandFailed(id, message));
      }

      return result;
    }

    if (action.type === "TREE/NODE_SELECTED") {
      const { id } = action.payload as { id: TreeNodeId };
      const result = next(action);

      try {
        if (!platform.isPlatformAvailable()) {
          throw new Error("Ultralight bridge is not available.");
        }
        const properties = platform.getFBXNodeProperties(id);
        storeApi.dispatch(treePropertiesLoaded(id, properties));
      } catch (err) {
        const message = err instanceof Error ? err.message : String(err);
        storeApi.dispatch(treePropertiesFailed(id, message));
      }

      return result;
    }

    return next(action);
  };
