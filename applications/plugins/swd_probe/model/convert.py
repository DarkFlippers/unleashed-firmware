#!/usr/bin/python

import plyfile
import argparse

parser = argparse.ArgumentParser(description='Convert a PLY file to C arrays.')
parser.add_argument('input_file', help='the input PLY file')
parser.add_argument('output_file', help='the output C file')
args = parser.parse_args()

# Open the PLY file
plydata = plyfile.PlyData.read(args.input_file)

# Extract the vertices
vertices = plydata['vertex'].data
num_vertices = len(vertices)

with open(args.output_file, 'w') as f:
    f.write('#define NUM_VERTICES %d\n' % num_vertices)
    f.write('float vertexCoords[NUM_VERTICES][3] = {\n')
    for i in range(num_vertices):
        x, y, z = vertices[i][0], vertices[i][1], vertices[i][2]
        f.write('  {%f, %f, %f},\n' % (x, y, z))
    f.write('};')

    # Extract the faces
    faces = plydata['face'].data
    num_faces = len(faces)
    f.write('int edgeIndices[][3] = {\n')
    for i in range(num_faces):
        face = faces[i][0]
        if len(face) == 3:
            f.write('  {%d, %d, %d},\n' % (face[0], face[1], face[2]))
        elif len(face) == 4:
            # Convert 4-index face to 2-index edges
            edges = [(face[0], face[1]), (face[1], face[2]), (face[2], face[3]), (face[3], face[0])]
            for edge in edges:
                f.write('  {%d, %d},\n' % (edge[0], edge[1]))
    f.write('};\n')
