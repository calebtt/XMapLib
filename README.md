# XMapLib
A closer to the metal library for Xbox controller to keyboard and mouse input.

<p>XMapLibSharp is a C# .NET GUI project using the C++ project code through a DLL. With this approach,
the project has access to the entire .NET framework for GUI work while keeping the native performance and power of the C++ XMapLib project code.
  <p>The project utilizes 
<ul>
  <li>.NET multi-threading</li> 
  <li>events</li> 
  <li>programmatic UI element creation</li> 
  <li>a producer/consumer design pattern.</li>
  <li>managed/native interop via custom C++ DLL</li>
</ul>

<a href="https://ibb.co/x28d2WX"><img src="https://i.ibb.co/0nVvnTm/XMap-Lib-gui1.jpg" alt="XMap-Lib-gui1" border="0"></a>

  <b><p>A high level code diagram of the XMapLib project code (native C++)</b>
<a href="https://ibb.co/vmjctWD"><img src="https://i.ibb.co/ZKMfZrN/XMap-Lib-uml.jpg" alt="XMap-Lib-uml" border="0"></a>

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
