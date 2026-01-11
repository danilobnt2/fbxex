export class FBXNodeProperty {
  name: string;
  type: string;
  value:
    | undefined
    | null
    | number
    | string
    | boolean
    | Array<number>
    | Array<Array<number>>;
}

export class FBXNodeAttribute {
  name: string;
  type: string;
  properties: Array<FBXNodeProperty>;
}

export class FBXNodeProps {
  name: string;
  properties: Array<FBXNodeProperty> = [];
  attributes: Array<FBXNodeAttribute> = [];
}

export class FBXNode {
  id: number;
  props: FBXNodeProps;

  constructor(id: number, props: FBXNodeProps = new FBXNodeProps()) {
    this.id = id;
    this.props = props;
  }
}
