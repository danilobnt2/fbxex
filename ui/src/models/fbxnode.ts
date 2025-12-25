export class NodePropertyValue {
    name: string;
    type: string;
    value: undefined
        | null
        | number 
        | string 
        | boolean 
        | Array<number>
        | Array<Array<number>>;
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