#define Version "3.0.0"
#define Name "Tunnel Comprehensive Collection Software"
#define Publisher "South"
#define URL "http://www.southsurvey.com/"

; 定义编译条件，取消注释其中一个
#define BuildClient
; #define BuildServer
#ifdef BuildClient
  #define ExeName "TCC Client "+Version+".exe"
  #define AppNameSuffix " Client"
  ; 客户端专用 AppId - 生成新的 GUID
  #define guid "{AEEE60E1-BE4D-48EB-9371-79A1FEAC8F0A}"
#else
  #define ExeName "TCC Server "+Version+".exe"
  #define AppNameSuffix " Server"
  ; 服务端专用 AppId - 生成新的 GUID
  #define guid "{B1234567-89AB-CDEF-0123-456789ABCDEF}"
#endif

[Setup]
; 注: AppId的值为单独标识该应用程序。
; 不要为其他安装程序使用相同的AppId值。
; (生成新的GUID，点击 工具|创建GUID 注册表)
AppId={{#guid}
AppName={#Name}{#AppNameSuffix}
AppVersion={#Version}
AppPublisher={#Publisher}
AppPublisherURL={#URL}
AppSupportURL={#URL}
AppUpdatesURL={#URL}
DisableDirPage=no
; 添加以下设置来确保使用新的安装路径
UsePreviousAppDir=no
; 修改默认安装路径
DefaultDirName={code:GetInstallDir}
; 如果想让用户可以选择安装盘符，可以使用:
;DefaultDirName={autopf}\{#Name}{#AppNameSuffix}{#Version}
DisableProgramGroupPage=yes
OutputDir=.\release
OutputBaseFilename={#Name}{#AppNameSuffix}{#Version}
SetupIconFile=.\assets\icons\logo.ico
Compression=lzma
SolidCompression=yes
[Code]
function GetInstallDir(Param: String): String;
begin
  if DirExists('D:\') then
    Result := 'D:\' + ExpandConstant('{#Name}{#AppNameSuffix}{#Version}')
  else
    Result := ExpandConstant('{autopf}\{#Name}{#AppNameSuffix}{#Version}');
end;

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl"
Name: "english"; MessagesFile: "compiler:Languages\English.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: checkablealone;
Name: "quicklaunchicon";Description:"{cm:CreateQuickLaunchIcon}"; GroupDescription:"{cm:AdditionalIcons}";Flags:checkablealone
Name: "startupicon"; Description: "开机启动"; GroupDescription: "{cm:AdditionalIcons}"; 

[Files]

; 根据编译条件选择不同的文件
#ifdef BuildClient
; 只包含主执行文件
Source: ".\out\build\release\out\bin\{#ExeName}"; DestDir: "{app}"; Flags: ignoreversion

Source: ".\out\build\release\out\bin\*.dll"; DestDir: "{app}";  Excludes: "*.pdb,*.lib,*.exp,*.ilk,*.log,bin\Qt*SerialPort.dll,test_*,debug_*,logs\*,temp\*,backup\*";Flags: ignoreversion recursesubdirs

#else
;Source: ".\out\build\release\out\bin\{#ExeName}"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: ".\out\build\release\out\*"; DestDir: "{app}"; Excludes: "*.pdb,*.lib,*.exp,*.ilk,*.log,bin\TCC*.exe,bin\Qt*Charts*.dll,bin\Qt*OpenGL*.dll,bin\Qt*Widgets.dll,test_*,debug_*,logs\*,temp\*,backup\*";Flags: ignoreversion recursesubdirs createallsubdirs
#endif

; 包含整个目录，但排除特定文件和目录
;Source: ".\out\build\release\out\*"; DestDir: "{app}"; Excludes: "*.pdb,*.lib,*.exp,*.ilk,*.log,*.tmp,test_*,debug_*,logs\*,temp\*,backup\*";Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
#ifdef BuildClient
Name: "{commonprograms}\{#Name}{#AppNameSuffix}{#Version}"; Filename: "{app}\{#ExeName}"
Name: "{commondesktop}\{#Name}{#AppNameSuffix}{#Version}"; Filename: "{app}\{#ExeName}"; Tasks: desktopicon
;在系统的 "启动" {commonstartup}文件夹 中创建一个快捷方式  
;修改为用户的启动{userstartup}文件夹
;{userstartup}: 当前用户的启动文件夹
;{commonstartup}: 所有用户的公共启动文件夹
;{userprograms}: 当前用户的程序文件夹
;{commonprograms}: 所有用户的公共程序文件夹
Name: "{userstartup}\{#Name}{#AppNameSuffix}{#Version}"; Filename: "{app}\{#ExeName}"; Tasks: startupicon
#else
Name: "{commonprograms}\{#Name}{#AppNameSuffix}{#Version}"; Filename: "{app}\bin\{#ExeName}"
Name: "{commondesktop}\{#Name}{#AppNameSuffix}{#Version}"; Filename: "{app}\bin\{#ExeName}"; Tasks: desktopicon
Name: "{userstartup}\{#Name}{#AppNameSuffix}{#Version}"; Filename: "{app}\bin\{#ExeName}"; Tasks: startupicon
#endif

[Run]
Filename: "{app}\bin\{#ExeName}"; Description: "{cm:LaunchProgram,{#StringChange(Name+AppNameSuffix, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

