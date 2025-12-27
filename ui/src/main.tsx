import React from "react";
import ReactDOM from "react-dom/client";

import { HeroUIProvider } from "@heroui/system";
import { Provider, useSelector } from "react-redux";

import "./main.css";

import ul from "./ul";
import { HierarchyTreeView, MenuBar } from "./containers";
import { AppState, createAppStore } from "./store";
import { Textarea } from "@heroui/input";
import { ReduxHierarchyTreeService, UltralightPlatformUtils, UltralightTreePlatform } from "./bindings";


declare global {
  interface Window {
    __remountApp?: () => void
    __ultralight?: typeof ul
  }
}


window.__ultralight = ul;


const treePlatform = new UltralightTreePlatform();
const store = createAppStore(treePlatform);
const DEFAULT_APP_VERSION = import.meta.env.VITE_APP_VERSION ?? "dev";

export function Root() {
  const [key, setKey] = React.useState(0)
  

  const platformUtils = React.useMemo(() => new UltralightPlatformUtils(), []);
  const hierarchyTreeService = React.useMemo(() => new ReduxHierarchyTreeService(), []);

  React.useEffect(() => {
    window.__remountApp = () => setKey(k => k + 1)
    return () => {
      delete window.__remountApp
    }
  }, [])

  const appVersion = ul.isAvailable ? ul.getAppVersion() : DEFAULT_APP_VERSION;
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
        <div className="app-shell text-slate-100">
          <MenuBar {...platformUtils} />
          <div className="app-main">
            <div className="sidebar">
              <HierarchyTreeView {...hierarchyTreeService} />
            </div>
            <div className="properties-pane">
              <Textarea
                classNames={{
                  base: "properties-base",
                  inputWrapper: "properties-wrapper",
                  innerWrapper: "properties-inner",
                  input: "properties-input"
                }}
                disableAutosize
                readOnly
                value={selectedNodePropsStr} />
            </div>
          </div>
          <div className="status-bar">
            <span className="status-item">fbxex {appVersion}</span>
            <span className="status-item">Ready</span>
          </div>
        </div>
      </HeroUIProvider>
    </React.StrictMode>
  )
}

const rootElement = document.getElementById("root");
if (rootElement) {
  ReactDOM.createRoot(rootElement).render(
    <Provider store={store}>
      <Root />
    </Provider>
  );
}
