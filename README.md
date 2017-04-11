# Modo Material Importer for Unreal Engine 4.15.1
  The Unreal Material Importer is a plug-in for Unreal Engine 4 that you can use to load Modo material .xml files and apply them to 3D meshes of a game level in the Unreal Engine 4 editor.
  
 Master is usually under heavy development. Binaries and source code on the master are for Unreal 4.15.1<br />
 
 Current Release that is identical to the one on UE4 Marketplace is here:<br />
 https://github.com/TheFoundry-Modo/UnrealPlugin/tree/UE4_15_0
 
 Older versions are zipped in other branches:<br />
 https://github.com/TheFoundry-Modo/UnrealPlugin/tree/legacy_unreal_versions<br />
 https://github.com/TheFoundry-Modo/UnrealPlugin/tree/UE4_13_0<br />
 https://github.com/TheFoundry-Modo/UnrealPlugin/tree/UE4_14_0
 
 For installation and other information, please read documentation here:<br />
 https://help.thefoundry.co.uk/modo/#help/pages/appendices/unreal_material_importer.html<br />
 
 Intended Platform (tested only): Desktop, Laptop, Windows, Mac OSX <br />
 _Linux users may need some tweak in .uplugin file, such as WhiteList to be able to build the plugin on Linux_ <br />
 
<br>Release Note 2016/12/9</br>
 * Users can import materials to any folder in the content browser.
 * Textures are imported into "Texture Sub-Path" which can be specified in the exporter.
 * Import and update only selected materials in the content browser.
 * Import and update only the materials that are used by selected meshes in the content browser.
 * Exporting with "Use Polygen Tag only", the dummy materials that are imported within FBX will be overwritten by the XML materials imported with the importer.
 * Exporting without "Use Polygen Tag only", the dummy materials that are imported within FBX will not be overwritten, but after importing XML materials, the meshes that are imported with FBX will use the XML materials instead of dummy materials.
 * Material Assignment is now applied to imported meshes, instead of instances in scene.
 * Users can use both MODO icon, or Unreal Engine 4 build-in import button to import materials.
 
<br>Release Note 2016/9/5</br>
 * Fix packaging issue in blueprint projects.
 * Accomplish the cfg file of the plugin.
 * Use better solutions for relative path.
 * Set WhiteList as "Win64" "Win32" "Mac".
 
<br>Release Note 2016/8/12</br>
 * Fix cook failure when packaging projects on Windows.
 * Fix name conversion, no more illegal characters.
 * Fix relative path issue.
 * Make the importer be able to load all supported texture formats.
 * Remove name prefix from imported textures.
 * Other minor bug fixes.
 
<br>Release Note 2016/5/17</br>
 * Fix random crash when applying materials.
 * Add a new feature to support importing images with color space (SRGB or not).
 * Update log to match 4.11.
