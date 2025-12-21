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
import { ITreePlatform, TreeNodeId, collapseNode, requestExpandNode } from "./viewmodels/treeview";


declare global {
  interface Window {
    __remountApp?: () => void
    __ultralight?: typeof ul
  }
}


window.__ultralight = ul;


const buildMessage = (rootChildren: number[] | null) => {
  if (rootChildren !== null) {
    return `Inspected FBX file with ${rootChildren.length} root children.`;   
  }
  return "No FBX file selected.";
}

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
}

class ReduxHierarchyTreeService implements IHierarchyTreeService {
  getNode = (state: AppState, id: TreeNodeId) => state.tree.nodes[id];
  createCollapseNodeAction = (id: TreeNodeId) => collapseNode(id);
  createRequestExpandNodeAction = (id: TreeNodeId) => requestExpandNode(id);
}

const treePlatform = new UltralightTreePlatform();
const store = createAppStore(treePlatform);

function Root() {
  const [key, setKey] = React.useState(0)
  const rootChildren = useSelector((state: AppState) => state.tree.nodes[state.tree.rootId].children);

  const platformUtils = React.useMemo(() => new UltralightPlatformUtils(), []);
  const hierarchyTreeService = React.useMemo(() => new ReduxHierarchyTreeService(), []);

  React.useEffect(() => {
    window.__remountApp = () => setKey(k => k + 1)
    return () => {
      delete window.__remountApp
    }
  }, [])

  let fileInspectedMessage = buildMessage(rootChildren);

  return (
    <React.StrictMode key={key}>
      <HeroUIProvider>
        <div className="h-screen flex flex-col text-slate-100 overflow-hidden min-h-0">
          <MenuBar {...platformUtils} />
          <div className="flex-1 flex overflow-hidden min-h-0">
            <div className="w-[360px] max-w-md min-w-[300px] h-full min-h-0 flex flex-col overflow-hidden">
              <HierarchyTreeView {...hierarchyTreeService} />
            </div>
            <div className="flex-1 flex items-center justify-center px-10 min-h-0">
              <div className="text-center">
                <p className="text-5xl font-bold">fbxex</p>
                <p className="text-2xl">The FBX Explorer and Inspector</p>
                <Spacer y={4} />
                <p>{fileInspectedMessage}</p>
              </div>
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
