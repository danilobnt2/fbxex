import { describe, expect, it, vi } from "vitest";

const mockUl = vi.hoisted(() => ({
  getFBXNode: vi.fn(() => ({
    props: {
      name: "Node 12",
      attributes: [
        { name: "Attr A", type: "Mesh", properties: [] },
        { name: "Attr B", type: "Light", properties: [] },
      ],
    },
  })),
}));

vi.mock("./ul", () => ({ default: mockUl }));

import { UltralightTreePlatform } from "./bindings";

describe("UltralightTreePlatform", () => {
  it("returns preview properties with attribute types", () => {
    const platform = new UltralightTreePlatform();

    const preview = platform.getFBXPreviewProperties(12);

    expect(mockUl.getFBXNode).toHaveBeenCalledWith(12);
    expect(preview).toEqual({
      name: "Node 12",
      attributeTypes: ["Mesh", "Light"],
    });
  });
});
