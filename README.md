# Modo Material Importer Unreal Plugin
The Unreal Material Importer is a plug-in for Unreal Engine 4 that you can use to load Modo material .xml files and apply them to 3D meshes of a game level in the Unreal Engine 4 editor.

Temporary branch for UE 4.13. Specifically for hot fixes.
 
 For installation and other information, please read documentation here:<br />
 https://help.thefoundry.co.uk/modo/#help/pages/appendices/unreal_material_importer.html<br />
 
 <br>Release Note 2016/9/05</br>
 Fix packaging issue in blueprint projects.<br />
 Accomplish the cfg file of the plugin.<br />
 Use better solutions for relative path.<br />
 Set WhiteList as "Win64" "Win32" "Mac".<br />
 
 <br>Release Note 2016/8/12</br>
 Fix cook failure when packaging projects on Windows.<br />
 Fix name conversion, no more illegal characters.<br />
 Fix relative path issue.<br />
 Make the importer be able to load all supported texture formats.<br />
 Remove name prefix from imported textures.<br />
 Other minor bug fixes.<br />
 
 <br>Release Note 2016/5/17</br>
 Fix random crash when applying materials.<br />
 Add a new feature to support importing images with color space (SRGB or not).<br />
 Update log to match 4.11.<br />
