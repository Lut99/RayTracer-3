# RayTracer-3: SceneLang specification

## 1. Introduction
The RayTracer is able to render large scene consisting of multiple objects. To easily define the relation between objects in the scene, where they are, how they move, etc, the SceneLang is introduced: a C-inspired markup language to define which objects are in the scene with which textures and where. Additionally, it also supports defining object animations, and users can use various data formats both in a .scene file and in external files to define meshes.

This document outlines how a .scene file looks like. First, we discuss the token language, which defines the elementary words out of which SceneLang is constructed. The grammar of the language follows in the second section, which together with the tokens forms the syntax. Finally, the last section specified some extra restrions, such as entity- or format-specific fields, what certain modifiers do, preprocessor language, etc.

## 2. Token language
As any properly defined language, the SceneLang defines a number of tokens used to abstract away from the text file and into the language's domain. This is also the first step in parsing, meaning that any syntax errors related to incorrect keywords or tokens will also appear first in your screen.

### Test
Test
