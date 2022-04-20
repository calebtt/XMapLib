# XMapLib
A closer to the metal library for Xbox controller to keyboard and mouse input.
<b><p>Current Features</b>
<ul>
  <li>Mapping of controller input to keyboard and mouse input on Windows.</li>
  <li>GUI for changing key bindings</li>
  <li>List of key binding presets to choose from</li>
  <li>Editable list of key bindings</li>
  <li>Adjustable mouse sensitivity</li>
  <li>Toggleable key repeat behavior</li>
 </ul>
 <b><p>Roadmap</b>
 <ul>
  <li>Lower CPU usage</li>
  <li>Portable input simulation thread priorities</li>
  <li>Dep. injected timestamp retrieval, system calls for high res. clock are heavy</li>
  <li>Make use of polar coordinates instead of scaling cartesian coordinates (PolarCode repo)</li>
 </ul>
 <b><p>Possible Ideas</b>
 <ul>
  <li>Integrate my PolarCode repo code</li>
  <li>Portable everything</li>
  <li>Run as a system service instead of on-demand app</li>
  <li>Load/Save config files and share them</li>
 </ul>
 
<p><i>XMapLibSharp is a C# .NET GUI project using the C++ project code through a DLL. With this approach,
the project has access to the entire .NET framework for GUI work while keeping the native performance and power of the C++ XMapLib project code.</i>
  <p>The project utilizes 
<ul>
  <li>.NET multi-threading</li> 
  <li>events</li> 
  <li>programmatic UI element creation</li> 
  <li>a producer/consumer design pattern.</li>
  <li>managed/native interop via custom C++ DLL</li>
</ul>

<a href="https://ibb.co/pymkwT7"><img src="https://i.ibb.co/M6XJSHv/XMap-Lib-gui2.png" alt="XMap-Lib-gui2" border="0"></a>
<a href="https://ibb.co/tsvcZ2S"><img src="https://i.ibb.co/FwyJsY9/XMap-Lib-gui3.png" alt="XMap-Lib-gui3" border="0"></a>
  
  <b><p>A high level code diagram of the XMapLib project code (native C++)</b>
  <a href="https://ibb.co/kBvqrRX"><img src="https://i.ibb.co/g9htX5J/XMap-Lib-uml3.png" alt="XMapLib-uml" border="0"></a>
  <p>The project utilizes
  <ul>
    <li>C++ templates</li>
    <li>Template SFINAE</li>
    <li>constexpr</li>
    <li>unit testing with Microsoft <a href="https://docs.microsoft.com/en-us/visualstudio/test/how-to-use-microsoft-test-framework-for-cpp?view=vs-2022">CppUnitTestingFramework</a></li>
    <li>multi-threading, many threads running concurrently and interacting</li> 
    <li>concurrency synchronization objects, like mutexes and RAII scoped locks</li>
    <li>C++ standard atomics</li>
    <li>lambdas, and passing lambdas as arguments</li>
    <li>usage of several Windows API functions including the XInput lib</li>
    <li>operator overloads</li>
    <li>input simulation</li>
    <li>C++ pointer wrappers like unique_ptr</li>
    <li>if-constexpr to do compile-time decisions instead of run-time</li>
    <li>STL containers and algorithms</li>
    <li>it is a real time system (or very close)</li>
    <li>lots and lots of object oriented programming</li>
    </ul>
    

  <b>The main classes for use in the native C++ XMapLib project are :</b>
 <ul>
<li><b><a href="https://github.com/calebtt/XMapLib/blob/master/XMapLib/MouseMapper.h">MouseMapper</a></b></li>
<li><b><a href="https://github.com/calebtt/XMapLib/blob/master/XMapLib/KeyboardMapper.h">KeyboardMapper</a></b></li>
<li><b><a href="https://github.com/calebtt/XMapLib/blob/master/XMapLib/MousePlayerInfo.h">MousePlayerInfo</a></b></li>
<li><b><a href="https://github.com/calebtt/XMapLib/blob/master/XMapLib/KeyboardPlayerInfo.h">KeyboardPlayerInfo</a></b></li>
<li>To set the mapping details <b><a href="https://github.com/calebtt/XMapLib/blob/master/XMapLib/KeyboardKeyMap.h">KeyboardKeyMap</a></b> encapsulates the information comprising a controller key to keyboard/mouse key mapping.</li>
  </ul>
<p>Also, please see the <b><a href="https://github.com/calebtt/XMapLib/blob/master/XMapLib/XMapLib.cpp">Example</a></b></p>

<b><p>The DLL API exposed from XMapLibDLL is described totally in <a href="https://github.com/calebtt/XMapLib/blob/master/XMapLibDLL/apifuncs.h">apifuncs.h</a> in the DLL project.</b>
