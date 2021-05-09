# RayTracer-3: SceneLang specification

## 1. Introduction
The RayTracer is able to render large scene consisting of multiple objects. To easily define the relation between objects in the scene, where they are, how they move, etc, the SceneLang is introduced: a C-inspired markup language to define which objects are in the scene with which textures and where. Additionally, it also supports defining object animations, and users can use various data formats both in a .scene file and in external files to define meshes.

This document outlines how a .scene file looks like. We do this in terms of grammar rules, where we first define the toplevel structure for each file, where we then zoom in on specific elements. Finally, we also discuss the behaviour of the standard preprocessor used for the SceneLang.

## 2. Top-level definitions
### Sections
The SceneLang defines one major structure at toplevel, which can be thought of as different sections of the file. Each of these sections contain specific kind of other statements, which we shall examine separately.  
Sections are defined by the following grammar rule:
```
<toplevel> := [<section>...]
<section>  := [<error_statement>...] <id> { [<statement>...] }
<id>       := /^[a-zA-Z_][a-zA-Z0-9_-]*$/
```
Here, ```<id>``` is a token used for defining identifiers, which must start with a letter or an underscore, and which can then contain any alphanumerical character, underscores or dashes. Also note that the SceneLang is newline and whitespace agnostic, and so the grammar rule can be formatted how they like.

Currently, three sections are supported in the SceneLang: ```data```, for defining meshes, textures or animations, ```entities```, where the conceptual objects can be defined, and ```global```, where re-useable expressions can be defined. To define either of those sections, use their name as the ID in the grammar rule for sections.

Note that, since sections define data, all of them can be defined multiple times in the same file or any of the file's dependency's. Conceptually, the parser will link all of these in one large file, and defines them in-order.

Finally, we have to talk about the ```<error_statement>```-tokens. Because the parser is quite verbose with warnings and errors, the error statements may be used to either generate or suppress certain warnings. The following grammar rules define how this works:
```
<error_statement> := @warning <id>
                  := @warning <string>
                  := @error <id>
                  := @error <string>
                  := @ignore <id>
```
The ```@warning```-statements generate a new warning when the parser encounters them, either by a warning ID (see Appendix A) or by a custom message given as string. The ```@error```-statements do the same, except that they throw fatal errors instead of warnings. Finally, the ```@ignore``` statement is used to suppress warnings with the given ID. Errors cannot be suppressed, as they are supposed to be fatal and thus prevent further parsing.

Note that ```@ignore```-statements have a limited scope of effect. They can only be defined above a section, statement or parameter, and work only for that element. Afterwards, the warning is not suppressed anymore and may thus freely occur again - unless they are ignored again, of course.

## 3. Statement language
### Statements - ```data``` section
Each section can then contain different statements. First, we will discuss those possible in the data sections. They are defined by the following grammar rules:
```
<statement> := [<error_statement>...] <format> <id> { <data> }
            := [<error_statement>...] extern <format> <id>: <path>;
<format>    := .obj
<data>      := /^((^[\\]*)|(\\[\\{}]))*$/
<path>      := <string>
<string>    := /^"((^["]*)|(\\[\\"nrt]))"$/
            := /^'((^[']*)|(\\[\\'nrt]))"$/
```
The ```.obj```-keyword defines the format used for the data given either as inline or as given by the specified file. Currently, the SceneLang only supports standard object files, but more may be added in the future.

Note that the ```<id>``` has to be unique for all statements across all data sections, since they will be used in the ```entities```-section to reference the data defiend here. The parser will thus throw errors if duplicate names are found. The names are sensitive to capitalization, of course (like all tokens).

The path given is supposed to be a valid string, and can be given either as an absolute path (starting with a slash or a drive letter followed by a colon) or as a relative path. Note, however, that the relative paths are not relative to the current working directory, but rather relative to the executable's location. This hopefully makes referencing files a bit easier.

### Statements - ```entities``` section
The other section the SceneLang currently supports is the ```entities```-section, which defines the conceptual entities which will be rendered. This means that the statements supported within it are different, and they look like:
```
<statement>   := [<error_statement>...] <entity_type> <id> { [<parameter>...] }
<entity_type> := triangle
              := sphere
              := object
<parameter>   := [<error_statement>...] <data_type> <id>: <expr>;
              := [<error_statement>...] data <id>: <format> <id>;
<data_type>   := bool
              := int
              := uint
              := float
              := vec3
```
Each of the entities is thus defined by an entity type, an identifier so that it can be referenced and a set of parameters that are used to define it. The parameters, in turn, can be thought of like keywords in a dictionary: each of them is defined as a key (an identifier) and a certain value, which can be an expression, a reference to something in the data section or a reference to another field from the same or another entity.

Although any identifier may be used, note that the SceneLang will not use all of them. Instead, for every entity type, a certain set of keywords exist which makes sense to define (see Appendix B). Any other parameters that you define will be ignored by the parser, unless they are referenced somewhere else. This last fact makes it possible to define "constants" which can be used across your file(s). Also see the global-section.

Expressions are constant values, given as one of the possible value types. However, to make life easier for the programmer, are complex mathmatical expressions supported to make life easier for the programmer. This results in the following set of grammar rules:
```
<expr>  := <const>
        := <monop> <expr>
        := <expr> <binop> <expr>
        := (<expr>)
        := (<data_type>) <expr>
        := <id>([<expr>...])
<const> := <bool>
        := <int>
        := <uint>
        := <float>
        := <vec3>
        := <ref>
<bool>  := true
        := false
<int>   := /^-?[0-9]+$/
<uint>  := /^[0-9]+$/
<float> := /^[0-9]*\.[0-9]+$/
        := /^[0-9]+e[0-9]+$/
<vec3>  := <float> <float> <float>
<ref>   := <id>.<id>
<monop> := -
        := +
        := !
        := ~
<binop> := +
        := -
        := *
        := /
        := %
        := &
        := |
        := ^
        := >>
        := <<
        := ==
        := !=
        := >
        := <
        := >=
        := <=
        := &&
        := ||
```
A careful observer may note that this is very similar to C-style expression syntax, which is indeed the case. In fact, the SceneLang aims to mimic its syntax, precedence and associativity as much as possible for consistency with the rest of C/C++ projects where the SceneLang is designed for. The only difference is the inclusion of vector operations and related build-in functions, which makes sense given the graphical nature of the SceneLang.

Note that the expressions in the SceneLang are strongly type-safe, and thus the compiler will give warnings and errors if it detects that the expression is typed incorrectly. To work around this problem, make use of the cast-operator, and check Appendix C to see a documentation of all build-in functions.

Also note that the cast-operator tries to mimic C-style convertions, but that this is of course undefined for the ```vec3``` data type. To this end, see Appendix D for a nice table that shows how each type is converted to each of the other types.

Finally, note that all types and constants are 32-bit. 64-bit support is not yet added, as this extra precision is rarely needed in graphical applications.

### Statements - ```global``` section
The global section can be used to define parameters that are not bound to any particular entity, but can instead be referenced by other entities, and thus promote re-useability. The global section is thus rather simple, and is only defined as a list of separate parameters:
```
<statement>      := <parameter>
```
To reference any of these parameters, use the identifier: ```global``` when referencing another field in another expression. Note that this means that entities cannot have ```global``` as their identifier.

## 4. The Preprocessor
To make life even easier for those managing the scene files, the SceneLang has a concept of a pre-processor, which can guide the parsing process. Most noteably, the preprocessor allows users to include other files, which will be (conceptually) copy/pasted in the original file at the location where they are included. The syntax for this is:
```
<macro> := #include <string>
```
The string defines the path of the other file, either as an absolute path or as a path relative to the current source file. Remember that the parser immediately starts reading data from the other file once included, which means that it's probably unwise to include inside sections, statements or expressions.

## 5. Final thoughts
By now, you should have a complete overview of how to create .scene files compatible by the SceneLang. Still in doubt? Remember that the parser is the golden oracle in this case, and so it often pays to just try something and see whether it works or not. This document can then be seen as an introduction in the global lines, where the parser will probably teach you the details.

----------------
# Appendix
## Appendix A: Warnings and Errors
This section of the appendix lists all warnings and errors that may occur in the parser, combined with their unique ID that can be used to throw or ignore them.

Note that warnings and errors have different scopes, and that it thus depends on the type of error statements used which are meant.

### Warnings

### Errors


## Appendix B: Reserved Entity Keywords
This section defines which parameter identifiers hold special meanings per entity. In effect, these are what the SceneLang is actually about, and so this list will probably be very useful.

### Triangle

### Sphere

### Object


## Appendix C: Build-in Functions
This sections documents the build-ins used to compute values at parse time.


## Appendix D: Cast Operator Behaviour
This section describes how the cast operator works for each possible pair of datatypes. This is shown in the following table:


