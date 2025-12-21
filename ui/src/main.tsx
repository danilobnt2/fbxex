import React from "react";
import ReactDOM from "react-dom/client";

import { HeroUIProvider } from "@heroui/system";
import { Spacer } from "@heroui/spacer";
import { Provider, useSelector } from "react-redux";

import "./index.css";

import ul from "./ul";
import { HierarchyTreeView, MenuBar } from "./containers";
import { AppState, store } from "./store";


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


function Root() {
  const [key, setKey] = React.useState(0)
  const rootChildren = useSelector((state: AppState) => state.tree.nodes[state.tree.rootId].children);

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
          <MenuBar />
          <div className="flex-1 flex overflow-hidden min-h-0">
            <div className="w-[360px] max-w-md min-w-[300px] h-full min-h-0 flex flex-col overflow-hidden">
              <HierarchyTreeView />
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
