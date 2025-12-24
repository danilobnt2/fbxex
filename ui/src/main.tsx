import React from "react";
import ReactDOM from "react-dom/client";

import { HeroUIProvider } from "@heroui/system";
import { Spacer } from "@heroui/spacer";
import { Provider, useSelector } from "react-redux";

import "./index.css";

import ul from "./ul";
import { HierarchyTreeView, MenuBar } from "./containers";
import { IPlatformUtils } from "./containers/menubar/menubar";
import { IHierarchyTreeService } from "./containers/htview/hierarchytreeview";
import { AppState, createAppStore, fileSelected } from "./store";
import { 
  ITreePlatform, 
  TreeNodeId, 
  collapseNode, 
  requestExpandNode, 
  selectNode } from "./viewmodels/treeview";
import { Textarea } from "@heroui/input";


declare global {
  interface Window {
    __remountApp?: () => void
    __ultralight?: typeof ul
  }
}


window.__ultralight = ul;


class UltralightPlatformUtils implements IPlatformUtils {
  isPlatformAvailable = () => ul.isAvailable;
  selectFbxFile = () => ul.selectFbxFile();
  closeWindow = () => ul.closeWindow();
  openAboutDialog = () => ul.openAboutDialog();
  createFileSelectedAction = () => fileSelected();
  onPlatformNotAvailable = () => alert("Ultralight not available.");
}

class UltralightTreePlatform implements ITreePlatform {
  isPlatformAvailable = () => ul.isAvailable;
  getFBXNodeChildren = (id: TreeNodeId) => ul.getFBXNodeChildren(id);
  getFBXNodeProperties = (id: TreeNodeId) => ul.getFBXNode(id).props;
  getFBXPreviewProperties = (id: TreeNodeId) => ({ 
    name: ul.getFBXNode(id).props.name });
}

class ReduxHierarchyTreeService implements IHierarchyTreeService {
  getNode = (state: AppState, id: TreeNodeId) => state.tree.nodes[id];
  createCollapseNodeAction = (id: TreeNodeId) => collapseNode(id);
  createRequestExpandNodeAction = (id: TreeNodeId) => requestExpandNode(id);
  createSelectNodeAction = (id: TreeNodeId) => selectNode(id);
}

const treePlatform = new UltralightTreePlatform();
const store = createAppStore(treePlatform);

function Root() {
  const [key, setKey] = React.useState(0)
  

  const platformUtils = React.useMemo(() => new UltralightPlatformUtils(), []);
  const hierarchyTreeService = React.useMemo(() => new ReduxHierarchyTreeService(), []);

  React.useEffect(() => {
    window.__remountApp = () => setKey(k => k + 1)
    return () => {
      delete window.__remountApp
    }
  }, [])

  let selectedNode = useSelector((state: AppState) => state.tree.selectedNodeId);
  let selectedNodeProps = useSelector((state: AppState) => 
    selectedNode ? state.tree.nodes[selectedNode].properties : null);
  var selectedNodePropsStr: string;
  try {
    selectedNodePropsStr = !selectedNodeProps 
      ? "{}" : 
      JSON.stringify(selectedNodeProps, null, 2);
  } catch (e) {
    selectedNodePropsStr = `{"error": "Error serializing properties: ${e}"}`;
  }

  return (
    <React.StrictMode key={key}>
      <HeroUIProvider>
        <div className="h-screen flex flex-col text-slate-100 overflow-hidden min-h-0">
          <MenuBar {...platformUtils} />
          <div className="flex-1 flex overflow-hidden min-h-0">
            <div className="w-[360px] max-w-md min-w-[300px] h-full min-h-0 flex flex-col overflow-hidden">
              <HierarchyTreeView {...hierarchyTreeService} />
            </div>
            <div className="flex-1 flex pl-2 pr-0 min-h-0 overflow-hidden">
              <Textarea
                className="flex-1 w-full h-full min-h-0"
                classNames={{
                  base: "w-full h-full flex-1 min-h-0",
                  inputWrapper: "w-full h-full min-h-0",
                  innerWrapper: "w-full h-full min-h-0",
                  input: "h-full min-h-0 overflow-auto"
                }}
                disableAutosize
                readOnly
                value={selectedNodePropsStr} />
            </div>
          </div>
        </div>
      </HeroUIProvider>
    </React.StrictMode>
  )
}

ReactDOM.createRoot(document.getElementById("root")!).render(
  <Provider store={store}>
    <Root />
  </Provider>
)
