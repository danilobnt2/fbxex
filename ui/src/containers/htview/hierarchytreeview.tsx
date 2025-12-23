import React from "react";
import { useDispatch, useSelector } from "react-redux";

import "./hierarchytreeview.css";

import { TreeNode, TreeNodeId } from "../../viewmodels/treeview";

export interface IHierarchyTreeService {
  getNode: (state: unknown, id: TreeNodeId) => TreeNode | undefined;
  createCollapseNodeAction: (id: TreeNodeId) => { type: string };
  createRequestExpandNodeAction: (id: TreeNodeId) => { type: string };
  createSelectNodeAction: (id: TreeNodeId) => { type: string };
}

type TreeNodeRowProps = {
  id: TreeNodeId;
  depth: number;
  rootId: TreeNodeId;
  tree: IHierarchyTreeService;
};

const TreeNodeRow: React.FC<TreeNodeRowProps> = ({ id, depth, rootId, tree }) => {
  const node = useSelector((state: unknown) => tree.getNode(state, id));
  const dispatch = useDispatch();

  if (!node) {
    return null;
  }

  const isRoot = id === rootId;
  const isLeaf = node.children !== null && node.children.length === 0;
  const isRootWaiting = isRoot && node.children === null && !node.isLoading && !node.error;
  const canToggle = !isLeaf && !isRootWaiting;
  const label = node.previewProperties?.name || `.$Node ${id}`;
  const indent = depth * 16;

  const handleToggle = () => {
    if (!canToggle) return;
    if (node.isExpanded) {
      dispatch(tree.createCollapseNodeAction(id));
    } else {
      dispatch(tree.createRequestExpandNodeAction(id));
    }
  };

  return (
    <div role="treeitem" aria-expanded={canToggle ? node.isExpanded : undefined} aria-level={depth + 1}>
      <div className="ht-row" style={{ paddingLeft: `${indent}px` }}>
        {canToggle ? (
          <button
            type="button"
            className="ht-expander"
            onClick={handleToggle}
            disabled={node.isLoading}
            aria-label={node.isExpanded ? "Collapse node" : "Expand node"}
          >
            {node.isExpanded ? "-" : "+"}
          </button>
        ) : (
          <span className="ht-expander" aria-hidden />
        )}

        <button
          type="button"
          className="ht-label"
          onClick={() => dispatch(tree.createSelectNodeAction(id))}
        >
          <span className="ht-node-name">
            {label}
            {isRootWaiting && " (select a file to load)"}
          </span>
          <span className="ht-node-id">#{node.id}</span>
          {node.isLoading && <span className="ht-status">Loading...</span>}
          {node.error && <span className="ht-status ht-status--error">{node.error}</span>}
          {isLeaf && <span className="ht-status ht-status--muted">No children</span>}
        </button>
      </div>

      {node.isExpanded && node.children && node.children.length > 0 && (
        <div role="group">
          {node.children.map((childId) => (
            <TreeNodeRow key={childId} id={childId} depth={depth + 1} rootId={rootId} tree={tree} />
          ))}
        </div>
      )}
    </div>
  );
};

export default function HierarchyTreeView(tree: IHierarchyTreeService) {
  const rootId = 0;
  const rootNode = useSelector((state: unknown) => tree.getNode(state, rootId));

  if (!rootNode) {
    return null;
  }

  return (
    <div className="hierarchy-tree-view">
      <div className="ht-header">
        <div className="ht-title">Hierarchy</div>
        <div className="ht-subtitle">Expand nodes to inspect the FBX structure.</div>
      </div>
      <div className="ht-tree">
        <TreeNodeRow id={rootId} depth={0} rootId={rootId} tree={tree} />
        {!rootNode.isLoading && !rootNode.error && rootNode.children === null && (
          <div className="ht-empty">Open an FBX file to populate the tree.</div>
        )}
      </div>
    </div>
  );
}
