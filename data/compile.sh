#!/bin/bash

glslangValidator -V vertexShader.vert -o vertexShader.spv
glslangValidator -V fragmentShader.frag -o fragmentShader.spv

glslangValidator -V fragmentShaderInclude.frag -Iinclude -o fragmentShaderInclude.spv
