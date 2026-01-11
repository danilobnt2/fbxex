import { IPlatformUtils } from "./containers/menubar/menubar";
import { IHierarchyTreeService } from "./containers/htview/hierarchytreeview";
import { AppState, fileSelected } from "./store";
import { ITreePlatform, TreeNodeId, collapseNode, requestExpandNode, selectNode } from "./viewmodels/treeview";
import ul from "./ul";

export class UltralightPlatformUtils implements IPlatformUtils {
  isPlatformAvailable = () => ul.isAvailable;
  selectFbxFile = () => ul.selectFbxFile();
  closeWindow = () => ul.closeWindow();
  openAboutDialog = () => ul.openAboutDialog();
  createFileSelectedAction = () => fileSelected();
  onPlatformNotAvailable = () => alert("Ultralight not available.");
}

export class UltralightTreePlatform implements ITreePlatform {
  isPlatformAvailable = () => ul.isAvailable;
  getFBXNodeChildren = (id: TreeNodeId) => ul.getFBXNodeChildren(id);
  getFBXNodeProperties = (id: TreeNodeId) => ul.getFBXNode(id).props;
  getFBXPreviewProperties = (id: TreeNodeId) => ({
    name: ul.getFBXNode(id).props.name,
  });
}

export class ReduxHierarchyTreeService implements IHierarchyTreeService {
  getNode = (state: AppState, id: TreeNodeId) => state.tree.nodes[id];
  createCollapseNodeAction = (id: TreeNodeId) => collapseNode(id);
  createRequestExpandNodeAction = (id: TreeNodeId) => requestExpandNode(id);
  createSelectNodeAction = (id: TreeNodeId) => selectNode(id);
}
