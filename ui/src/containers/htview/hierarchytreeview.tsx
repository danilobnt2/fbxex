import React from "react";
import { Button } from "@heroui/button";
import { Card, CardBody, CardHeader } from "@heroui/card";
import { Chip } from "@heroui/chip";
import { ScrollShadow } from "@heroui/scroll-shadow";
import { Spinner } from "@heroui/spinner";
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
  const attributeTypes = node.previewProperties?.attributeTypes?.length ? node.previewProperties.attributeTypes.join(", ") : null;
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
    <div
      role="treeitem"
      aria-expanded={canToggle ? node.isExpanded : undefined}
      aria-level={depth + 1}
      aria-selected={node.isSelected || undefined}
    >
      <div className="ht-row" style={{ paddingLeft: `${indent}px` }}>
        {canToggle ? (
          <Button
            isIconOnly
            size="sm"
            variant="light"
            radius="sm"
            className="ht-expander"
            onPress={handleToggle}
            isDisabled={node.isLoading}
            aria-label={node.isExpanded ? "Collapse node" : "Expand node"}
          >
            {node.isExpanded ? "−" : "+"}
          </Button>
        ) : (
          <Button
            isIconOnly
            size="sm"
            variant="light"
            radius="sm"
            className="ht-expander"
            isDisabled
            aria-hidden
          >
            <span className="ht-expander-placeholder" />
          </Button>
        )}

        <Button
          size="sm"
          variant={node.isSelected ? "flat" : "light"}
          radius="sm"
          className={`ht-label ${node.isSelected ? "ht-label--selected" : ""}`}
          onPress={() => dispatch(tree.createSelectNodeAction(id))}
        >
          <div className="ht-label-text">
            <span className="ht-node-name">
              {label}
              {isRootWaiting && " (select a file to load)"}
            </span>
            {attributeTypes && <span className="ht-node-attributes">{attributeTypes}</span>}
            <span className="ht-node-id">[{node.id}]</span>
          </div>

          <div className="ht-statuses">
            {node.isLoading && (
              <Chip
                size="sm"
                variant="flat"
                color="primary"
                className="ht-chip"
                startContent={<Spinner size="sm" color="primary" className="ht-spinner" />}
              >
                Loading
              </Chip>
            )}
            {node.error && (
              <Chip size="sm" variant="flat" color="danger" className="ht-chip">
                {node.error}
              </Chip>
            )}
            {isLeaf && !node.isLoading && !node.error && (
              <Chip size="sm" variant="flat" color="default" className="ht-chip">
                No children
              </Chip>
            )}
          </div>
        </Button>
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
    <Card className="hierarchy-tree-view ht-card" radius="none" shadow="none">
      <CardHeader className="ht-header">
        <div className="ht-title">Hierarchy</div>
        <div className="ht-subtitle">Expand nodes to inspect the FBX structure.</div>
      </CardHeader>
      <CardBody className="ht-body">
        <ScrollShadow className="ht-tree">
          <div role="tree" aria-label="FBX hierarchy">
            <TreeNodeRow id={rootId} depth={0} rootId={rootId} tree={tree} />
            {!rootNode.isLoading && !rootNode.error && rootNode.children === null && (
              <div className="ht-empty">Open an FBX file to populate the tree.</div>
            )}
          </div>
        </ScrollShadow>
      </CardBody>
    </Card>
  );
}
