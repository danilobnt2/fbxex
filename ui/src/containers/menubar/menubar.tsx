import React from "react";
import { Button } from "@heroui/button";
import { Dropdown, DropdownTrigger, DropdownMenu, DropdownItem } from "@heroui/dropdown";
import { Navbar, NavbarContent, NavbarItem } from "@heroui/navbar";
import { Key } from "@react-types/shared";
import { useDispatch } from "react-redux";

import "./menubar.css";

export interface IPlatformUtils {
  isPlatformAvailable: () => boolean;
  selectFbxFile: () => boolean;
  closeWindow: () => void;
  openAboutDialog: () => void;
  createFileSelectedAction: () => { type: string };
  onPlatformNotAvailable?: () => void;
}

export default function MenuBar(platform: IPlatformUtils) {
  const dispatch = useDispatch();
  const handlePlatformNotAvailable = (
    platform.onPlatformNotAvailable ?? (() => alert("Platform not available.")));

  const handleOpen = () => {
    if (!platform.isPlatformAvailable()) {
      handlePlatformNotAvailable();
      return;
    }
    const selected = platform.selectFbxFile();
    if (selected) {
      dispatch(platform.createFileSelectedAction());
    }
  };

  const handleFileMenuAction = React.useCallback((key: Key) => {
    if (key === "open") {
      handleOpen();
      return;
    }
    if (key === "close") {
      if (!platform.isPlatformAvailable()) {
        handlePlatformNotAvailable();
        return;
      }
      platform.closeWindow();
    }
  }, [platform, handlePlatformNotAvailable, handleOpen]);

  const handleHelpMenuAction = React.useCallback((key: Key) => {
    if (key !== "about") return;
    if (!platform.isPlatformAvailable()) {
      handlePlatformNotAvailable();
      return;
    }
    platform.openAboutDialog();
  }, [platform, handlePlatformNotAvailable]);

  return (
    <Navbar
      height="2rem"
      maxWidth="full"
      classNames={{
        base: "menu-bar",
        wrapper: "max-w-full px-0 min-h-0 h-8",
        content: "menu-content",
        item: "menu-navbar-item",
      }}
    >
      <NavbarContent justify="start" className="menu-content">
        <Dropdown>
          <NavbarItem className="menu-navbar-item">
            <DropdownTrigger>
              <Button
                disableRipple
                size="sm"
                variant="light"
                className="menu-trigger"
              >
                File
              </Button>
            </DropdownTrigger>
          </NavbarItem>
          <DropdownMenu
            aria-label="File menu"
            onAction={handleFileMenuAction}
            className="menu-dropdown"
            itemClasses={{
              base: "drop-item",
            }}
          >
            <DropdownItem key="open">Open ...</DropdownItem>
            <DropdownItem key="close">Close</DropdownItem>
          </DropdownMenu>
        </Dropdown>

        <Dropdown>
          <NavbarItem className="menu-navbar-item">
            <DropdownTrigger>
              <Button
                disableRipple
                size="sm"
                variant="light"
                className="menu-trigger"
              >
                Help
              </Button>
            </DropdownTrigger>
          </NavbarItem>
          <DropdownMenu
            aria-label="Help menu"
            onAction={handleHelpMenuAction}
            className="menu-dropdown"
            itemClasses={{
              base: "drop-item",
            }}
          >
            <DropdownItem key="about">About</DropdownItem>
          </DropdownMenu>
        </Dropdown>
      </NavbarContent>
    </Navbar>
  );
}
