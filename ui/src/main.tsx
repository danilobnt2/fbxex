import React from "react";
import ReactDOM from "react-dom/client";

import { HeroUIProvider } from "@heroui/system";
import { Spacer } from "@heroui/spacer";
import { Provider, useSelector } from "react-redux";

import "./index.css";

import ul from "./ul";
import { MenuBar } from "./containers";
import { AppState, store } from "./store";


declare global {
  interface Window {
    __remountApp?: () => void
    __ultralight?: typeof ul
  }
}


window.__ultralight = ul;


const getFbxFileFormatVersion = () => 
  ul.isAvailable ? ul.getFbxFileFormatVersion() : "🤷";


const buildMessage = (rootChildren: number[] | null) => {
  if (rootChildren !== null) {
    return `Inspected FBX file with ${rootChildren.length} root children.`;   
  }
  return "No FBX file selected.";
}


function Root() {
  const [key, setKey] = React.useState(0)
  const rootChildren = useSelector((state: AppState) => state.rootChildren)

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
        <div className="h-screen flex flex-col">
          <MenuBar />
          <div className="flex-1 flex items-center justify-center">
            <div className="text-center">
              <p className="text-5xl font-bold">fbxex</p>
              <p className="text-2xl">The FBX Explorer and Inspector</p>
              <Spacer y={4} />
              <p>FBX file format version {getFbxFileFormatVersion()}</p>
              <p>{fileInspectedMessage}</p>
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
