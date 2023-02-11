import qbs
import qbs.Process
import qbs.File
import qbs.FileInfo
import qbs.TextFile
import "../../../libs/openFrameworksCompiled/project/qtcreator/ofApp.qbs" as ofApp

Project{
    property string of_root: '../../..'

    ofApp {
        name: { return FileInfo.baseName(sourceDirectory) }

        files: [
            'src/main.cpp',
            'src/ofApp.cpp',
            'src/ofApp.h',
        ]

        of.addons: [
            'ofxVisualProgramming'
        ]

        // additional flags for the project. the of module sets some
        // flags by default to add the core libraries, search paths...
        // this flags can be augmented through the following properties:
        of.pkgConfigs: []       // list of additional system pkgs to include
        of.includePaths: []     // include search paths
        of.cFlags: [""]           // flags passed to the c compiler
        of.cxxFlags: []         // flags passed to the c++ compiler
        of.linkerFlags: []      // flags passed to the linker
        of.defines: []          // defines are passed as -D to the compiler
        // and can be checked with #ifdef or #if in the code
        of.frameworks: []       // osx only, additional frameworks to link with the project
        of.staticLibraries: []  // static libraries
        of.dynamicLibraries: [] // dynamic libraries

        // other flags can be set through the cpp module: http://doc.qt.io/qbs/cpp-module.html
        // eg: this will enable ccache when compiling
        //
        // cpp.compilerWrapper: 'ccache'

        // Include ofxSyphon on osx
        Properties {
            // osx only, additional frameworks to link with the project
            condition: qbs.targetOS.contains("osx") || qbs.targetOS.contains("macos")
            of.addons: outer.concat(['ofxSyphon'])
            of.frameworks: outer.concat(['Syphon'])
            cpp.frameworkPaths: ['../../../addons/ofxSyphon/libs/Syphon/lib/osx/']
            // dirty fix for compiling .mm files (not auto-detected on qt)
            files: outer.concat([
                                    '../../../addons/ofxSyphon/src/ofxSyphonClient.mm',
                                    '../../../addons/ofxSyphon/src/ofxSyphonServer.mm',
                                    '../../../addons/ofxSyphon/src/ofxSyphonServerDirectory.mm',
                                    '../../../addons/ofxSyphon/libs/Syphon/src/SyphonNameboundClient.m',
                                ])
        }

        Depends{
            name: "cpp"
        }

        // common rules that parse the include search paths, core libraries...
        Depends{
            name: "of"
        }

        // dependency with the OF library
        Depends{
            name: "openFrameworks"
        }
    }

    property bool makeOF: true  // use makfiles to compile the OF library
    // will compile OF only once for all your projects
    // otherwise compiled per project with qbs


    property bool precompileOfMain: false  // precompile ofMain.h
    // faster to recompile when including ofMain.h
    // but might use a lot of space per project

    references: [FileInfo.joinPaths(of_root, "/libs/openFrameworksCompiled/project/qtcreator/openFrameworks.qbs")]
}
