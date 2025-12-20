export class FBXNodeProps {}

export class FBXNode {
    id: number;
    props: FBXNodeProps;

    constructor(id: number, props: FBXNodeProps = new FBXNodeProps()) {
        this.id = id;
        this.props = props;
    }
}