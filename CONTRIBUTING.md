# Contribution Guidelines

We're really glad you're interested in helping with implementing features into libfinite! This is a big project and help from our developers makes this process easier. Below are some general rules you should follow while attempting to contribute to the project to ensure that all developers have a smooth transition to libfinite from version to version

## New Functions

In general, we're pretty lenient on new functions and implementations of new features per community requests. As long as there a reasonable use-case for a specific feature and it doesn't contridiction with our minimal system rules (see below) we tend accept it in. However ensure that your functions follow the function family naming convention.

The function family naming convention is a snake cased naming system we use where the function family appears at the beginning of the name of the function.
For example the FiniteDraw family has `finite_draw` as the prefix to all of its functions.

## New Function Families

We generally discourage creating new function families. Function Families should only be made for unique features that don't snuggly fit into one of the existing families. For example a gamepad function could in therory belong to a `FiniteGamepad` family but since it's input the more general `FiniteInput` family makes more sense.

Basically, if a feature can fit into one of the function families, create a subfamily. In our example above the `FiniteGamepad` subfamily is apart of the `FiniteInput` family.

When creating a new family, there should generally be one "central" struct thats associated with family, although exceptions may be made depending on what's being implemented. If you're unsure, just ask in the Cubix Discord Server.

## Macros

All public facing functions should be debug functions with a macro redefintion that quietly passes the debug paramaters. As such, outside of the expected implementations, Macros should be avoided to void overwriting. Additionally, macros that mask the behavior of non-libfinite functions is prohibited (For example a sleep() wrapper called wait()).

Should a macro be absolutely neccessary, it should be mentioned in your pull request notes as well as an explaination as to why it is necesary.

## Commits

While we don't enforce any specific naming convention, please attempt to briefly what changes are being made in the commit message. Additionally commit details are optional.

## The Minimal System Rules

1) In general, avoid new dependencies (within reason).
2) Do not add conflicting dependencies that replicate behavior already implemented by libfinite or system functions directly. (For example, GLFW should not be used since FiniteRender achieves effectively the same thing.)
3) Do not implement any dependency that is not Open Source or are not built with linux in mind.
4) Avoid adding new dependencies that are not mentioned in the Beyond Linux From Scratch documentation
5) Link the source code to any new dependencies in your pull requests.
