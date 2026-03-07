import React from "react";

import { HeroUIProvider } from "@heroui/system";
import { useSelector } from "react-redux";
import { Textarea } from "@heroui/input";

import ul from "./ul";
import { HierarchyTreeView, MenuBar } from "./containers";
import { AppState } from "./store";
import { ReduxHierarchyTreeService, UltralightPlatformUtils } from "./bindings";

const DEFAULT_APP_VERSION = import.meta.env.VITE_APP_VERSION ?? "dev";

export function Root() {
  const [key, setKey] = React.useState(0);

  const platformUtils = React.useMemo(() => new UltralightPlatformUtils(), []);
  const hierarchyTreeService = React.useMemo(() => new ReduxHierarchyTreeService(), []);

  React.useEffect(() => {
    window.__remountApp = () => setKey((k) => k + 1);
    return () => {
      delete window.__remountApp;
    };
  }, []);

  const appVersion = ul.isAvailable ? ul.getAppVersion() : DEFAULT_APP_VERSION;
  const fbxFormat = useSelector((state: AppState) => state.loadedFileFormat);
  const selectedNode = useSelector((state: AppState) => state.tree.selectedNodeId);
  const selectedNodeProps = useSelector((state: AppState) =>
    selectedNode ? state.tree.nodes[selectedNode].properties : null
  );
  let selectedNodePropsStr: string;
  try {
    selectedNodePropsStr = !selectedNodeProps ? "{}" : JSON.stringify(selectedNodeProps, null, 2);
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
                  input: "properties-input",
                }}
                disableAutosize
                readOnly
                value={selectedNodePropsStr}
              />
            </div>
          </div>
          <div className="status-bar">
            <span className="status-item">fbxex {appVersion}</span>
            <span className="status-item">{fbxFormat ? `File format: ${fbxFormat}` : "Ready"}</span>
          </div>
        </div>
      </HeroUIProvider>
    </React.StrictMode>
  );
}
