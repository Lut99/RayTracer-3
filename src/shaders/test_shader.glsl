/* VERTEX SHADER
 *   by Lut99
 *
 * Shader for mapping the vertices from world space to the output frame space.
 */

#version 450

layout (local_size_x = 32) in;

layout(set = 0, binding = 0) buffer InputBuffer {
    uint[1024] elems;
} copyfrom;
layout(set = 0, binding = 1) buffer OutputBuffer {
    uint[1024] elems;
} copyto;

void main() 
{		
    //grab global ID
	uint gID = gl_GlobalInvocationID.x;

    // Do the copy
    copyto.elems[gID] = copyfrom.elems[gID];
}
