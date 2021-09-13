from enum import Enum
from collections import OrderedDict


class FsNode:
    class Type(Enum):
        File = 0
        Directory = 1

    def __init__(self, name: str, type: "FsNode.Type", **kwargs):
        self.name = name
        self.type = type
        self.data = kwargs
        self.parent = None
        self.children = OrderedDict()

    def addChild(self, node: "FsNode"):
        self.children[node.name] = node
        node.parent = self

    def addDirectory(self, path):
        fragments = path.split("/")
        name = fragments[-1]
        fragments = fragments[:-1]
        parent_node = self.traverse(fragments)
        if not parent_node:
            raise Exception(f"No parent node found for: {path}")
        parent_node.addChild(FsNode(name, FsNode.Type.Directory))

    def addFile(self, path, md5, size):
        fragments = path.split("/")
        name = fragments[-1]
        fragments = fragments[:-1]
        parent_node = self.traverse(fragments)
        if not parent_node:
            raise Exception(f"No parent node found for: {path}")
        parent_node.addChild(FsNode(name, FsNode.Type.File, md5=md5, size=size))

    def getChild(self, name):
        return self.children[name]

    def traverse(self, fragments):
        current = self
        for fragment in fragments:
            current = current.getChild(fragment)
            if not current:
                break
        return current

    def getPath(self):
        fragments = []
        current = self
        while current.parent:
            fragments.append(current.name)
            current = current.parent
        return "/".join(reversed(fragments))

    def dump(self):
        ret = {}
        ret["name"] = (self.name,)
        ret["type"] = (self.type,)
        ret["path"] = (self.getPath(),)
        if len(self.children):
            ret["children"] = [node.dump() for node in self.children.values()]
        return ret


def compare_fs_trees(left: FsNode, right: FsNode):
    # import pprint
    # pprint.pprint(left.dump())
    # pprint.pprint(right.dump())

    only_in_left = []
    changed = []
    only_in_right = []
    return [], [], []
