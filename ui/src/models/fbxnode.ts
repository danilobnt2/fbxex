class VectorXYZProperty {
    x: number;
    y: number;
    z: number;
}

class VectorWXYZProperty {
    x: number;
    y: number;
    z: number;
    w: number;
}

class VectorUVProperty {
    u: number;
    v: number;
}

class ColorRGBProperty {
    r: number;
    g: number;
    b: number;
}

class ColorRGBAProperty {
    r: number;
    g: number;
    b: number;
    a: number;
}

class ColorHSBProperty {
    h: number;
    s: number;
    b: number;
}

export class NodePropertyValue {
    name: string;
    type: string;
    value: undefined
        | null
        | number 
        | string 
        | boolean 
        | Array<number>
        | Array<Array<number>>
        | VectorXYZProperty 
        | VectorWXYZProperty
        | VectorUVProperty
        | ColorRGBProperty 
        | ColorRGBAProperty
        | ColorHSBProperty
}

export class FBXNodeProps {
    name: string;
    properties: Record<string, NodePropertyValue>;
}

export class FBXNode {
    id: number;
    props: FBXNodeProps;

    constructor(id: number, props: FBXNodeProps = new FBXNodeProps()) {
        this.id = id;
        this.props = props;
    }
}