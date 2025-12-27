import { describe, expect, it } from "vitest";

import { FBXNode, FBXNodeProps, NodePropertyValue } from "./fbxnode";

describe("FBXNode", () => {
  it("stores the provided props and preserves property values", () => {
    const props = new FBXNodeProps();
    props.name = "RootNode";
    props.properties = {
      translation: { name: "Lcl Translation", type: "double3", value: [1, 2, 3] },
      visibility: { name: "Visibility", type: "bool", value: true },
      matrix: {
        name: "Transform",
        type: "matrix",
        value: [
          [1, 0, 0],
          [0, 1, 0],
          [0, 0, 1],
        ],
      },
    } as Record<string, NodePropertyValue>;

    const node = new FBXNode(42, props);

    expect(node.id).toBe(42);
    expect(node.props).toBe(props);
    expect(node.props.name).toBe("RootNode");
    expect(node.props.properties?.translation.value).toEqual([1, 2, 3]);
    expect(node.props.properties?.matrix.value).toEqual([
      [1, 0, 0],
      [0, 1, 0],
      [0, 0, 1],
    ]);
  });

  it("creates a new props container when none is provided", () => {
    const node = new FBXNode(7);

    expect(node.id).toBe(7);
    expect(node.props).toBeInstanceOf(FBXNodeProps);
    expect(node.props.name).toBeUndefined();
    expect(node.props.properties).toBeUndefined();
  });

  it("does not share default props between node instances", () => {
    const first = new FBXNode(1);
    const second = new FBXNode(2);

    first.props.name = "First Node";

    expect(first.props).not.toBe(second.props);
    expect(second.props.name).toBeUndefined();
  });
});
