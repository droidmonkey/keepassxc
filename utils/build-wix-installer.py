#!/usr/bin/env python3
"""
KeePassXC WiX Installer Build Script
Copyright (C) 2024 KeePassXC team <https://keepassxc.org/>

This script builds Windows installers using WiX Toolset v4+ directly,
bypassing CPack for more control and flexibility.

Usage:
    python build-wix-installer.py [options]
    
Options:
    --build-dir DIR       Build directory (default: current directory)
    --source-dir DIR      Source directory (default: parent of build dir)
    --output-dir DIR      Output directory for MSI (default: build dir)
    --version VERSION     Version string (e.g., 2.7.9)
    --arch ARCH          Architecture: x64, x86, arm64 (default: x64)
    --help               Show this help message
"""

import argparse
import os
import sys
import subprocess
import json
import re
from pathlib import Path


class WixBuilder:
    """Build Windows installer using WiX Toolset v4+"""
    
    def __init__(self, args):
        self.build_dir = Path(args.build_dir).resolve()
        self.source_dir = Path(args.source_dir).resolve()
        self.output_dir = Path(args.output_dir).resolve()
        self.version = args.version
        self.arch = args.arch
        self.verbose = args.verbose
        
        # Try to read version from CMakeCache.txt if not provided
        if not self.version:
            self.version = self.read_version_from_cmake()
            if not self.version:
                self.log("Version not specified and could not be read from CMakeCache.txt", "ERROR")
                sys.exit(1)
        
        # App configuration
        self.app_name = "KeePassXC"
        self.upgrade_guid = "88785A72-3EAE-4F29-89E3-BC6B19BA9A5B"
        self.vendor = "KeePassXC Team"
        
        # Paths
        self.install_root = self.build_dir / "src"
        self.windows_share = self.source_dir / "share" / "windows"
        
        # WiX files
        self.wix_template = self.windows_share / "wix-template.xml"
        self.wix_patch = self.windows_share / "wix-patch.xml"
        self.wix_extra_sources = [
            self.windows_share / "KPXC_InstallDir.wxs",
            self.windows_share / "KPXC_InstallDirDlg.wxs",
            self.windows_share / "KPXC_ExitDlg.wxs"
        ]
        
        # Resources
        self.product_icon = self.windows_share / "keepassxc.ico"
        self.ui_banner = self.windows_share / "installer-banner.png"
        self.ui_dialog = self.windows_share / "installer-wizard.png"
        
        # Output
        self.output_msi = self.output_dir / f"{self.app_name}-{self.version}-Win64.msi"
        
    def read_version_from_cmake(self):
        """Read version from CMakeCache.txt"""
        cmake_cache = self.build_dir / "CMakeCache.txt"
        if cmake_cache.exists():
            try:
                with open(cmake_cache, 'r') as f:
                    for line in f:
                        if line.startswith("KEEPASSXC_VERSION:"):
                            parts = line.split("=")
                            if len(parts) == 2:
                                return parts[1].strip()
            except Exception as e:
                self.log(f"Could not read CMakeCache.txt: {e}", "WARN")
        return None
        
    def log(self, message, level="INFO"):
        """Print log message"""
        if level == "DEBUG" and not self.verbose:
            return
        prefix = f"[{level}]"
        print(f"{prefix} {message}")
        
    def run_command(self, cmd, cwd=None, capture=False):
        """Run a command and handle errors"""
        self.log(f"Running: {' '.join(cmd)}", "DEBUG")
        try:
            if capture:
                result = subprocess.run(
                    cmd,
                    cwd=cwd or self.build_dir,
                    check=True,
                    capture_output=True,
                    text=True
                )
                return result.stdout
            else:
                subprocess.run(
                    cmd,
                    cwd=cwd or self.build_dir,
                    check=True
                )
        except subprocess.CalledProcessError as e:
            self.log(f"Command failed: {e}", "ERROR")
            if hasattr(e, 'stderr') and e.stderr:
                self.log(f"Error output: {e.stderr}", "ERROR")
            sys.exit(1)
            
    def find_wix_exe(self):
        """Find wix.exe in the system"""
        # Check environment variable
        wix_env = os.environ.get("WIX")
        if wix_env:
            wix_exe = Path(wix_env) / "bin" / "wix.exe"
            if wix_exe.exists():
                return wix_exe
                
        # Check common installation paths
        program_files = [
            os.environ.get("ProgramFiles", "C:\\Program Files"),
            os.environ.get("ProgramFiles(x86)", "C:\\Program Files (x86)")
        ]
        
        for pf in program_files:
            for version in ["6.0", "5.0", "4.0"]:
                wix_exe = Path(pf) / f"WiX Toolset v{version}" / "bin" / "wix.exe"
                if wix_exe.exists():
                    return wix_exe
                    
        # Try PATH
        try:
            result = subprocess.run(
                ["where", "wix.exe"],
                capture_output=True,
                text=True,
                check=False
            )
            if result.returncode == 0:
                return Path(result.stdout.strip().split('\n')[0])
        except Exception:
            pass
            
        self.log("Could not find wix.exe. Please install WiX Toolset v4+ or set the WIX environment variable.", "ERROR")
        sys.exit(1)
        
    def get_wix_version(self, wix_exe):
        """Get WiX version"""
        try:
            output = self.run_command([str(wix_exe), "--version"], capture=True)
            match = re.search(r"wix\.exe version ([0-9]+\.[0-9]+\.[0-9]+)", output)
            if match:
                return match.group(1)
        except Exception:
            pass
        return "unknown"
        
    def verify_files(self):
        """Verify all required files exist"""
        self.log("Verifying required files...")
        
        files_to_check = [
            ("WiX template", self.wix_template),
            ("WiX patch", self.wix_patch),
            ("Product icon", self.product_icon),
            ("UI banner", self.ui_banner),
            ("UI dialog", self.ui_dialog)
        ]
        
        for name, path in files_to_check:
            if not path.exists():
                self.log(f"{name} not found: {path}", "ERROR")
                sys.exit(1)
                
        for source in self.wix_extra_sources:
            if not source.exists():
                self.log(f"WiX source not found: {source}", "ERROR")
                sys.exit(1)
                
        if not self.install_root.exists():
            self.log(f"Install directory not found: {self.install_root}", "ERROR")
            self.log("Please build the project first.", "ERROR")
            sys.exit(1)
            
        self.log("All required files found.")
        
    def create_wix_variables_file(self):
        """Create WiX variables include file"""
        self.log("Creating WiX variables file...")
        
        variables_file = self.build_dir / "cpack_variables.wxi"
        
        # Clean version without suffixes
        version_clean = re.sub(r'-.*$', '', self.version)
        
        # Generate a product GUID (should be unique for each version)
        # For simplicity, we'll use a deterministic GUID based on version
        # In production, this should be generated properly
        product_guid = "*"  # Auto-generate
        
        content = f"""<?xml version="1.0" encoding="UTF-8"?>
<Include>
  <SetProperty Id="CPACK_PACKAGE_NAME" Value="{self.app_name}" />
  <SetProperty Id="CPACK_PACKAGE_VENDOR" Value="{self.vendor}" />
  <SetProperty Id="CPACK_PACKAGE_VERSION" Value="{version_clean}" />
  <SetProperty Id="CPACK_WIX_PRODUCT_GUID" Value="{product_guid}" />
  <SetProperty Id="CPACK_WIX_UPGRADE_GUID" Value="{self.upgrade_guid}" />
  <SetProperty Id="CPACK_WIX_LICENSE_RTF" Value="{self.build_dir / 'INSTALLER_LICENSE.txt'}" />
  <SetProperty Id="CPACK_WIX_PRODUCT_ICON" Value="{self.product_icon}" />
  <SetProperty Id="CPACK_WIX_UI_BANNER" Value="{self.ui_banner}" />
  <SetProperty Id="CPACK_WIX_UI_DIALOG" Value="{self.ui_dialog}" />
  <SetProperty Id="CPACK_WIX_UI_REF" Value="KPXC_InstallDir" />
</Include>
"""
        
        variables_file.write_text(content)
        self.log(f"Created {variables_file}")
        return variables_file
        
    def create_license_file(self):
        """Create license file for installer"""
        license_source = self.source_dir / "LICENSE.GPL-2"
        license_dest = self.build_dir / "INSTALLER_LICENSE.txt"
        
        if not license_dest.exists() and license_source.exists():
            self.log("Copying license file...")
            import shutil
            shutil.copy2(license_source, license_dest)
            
    def find_heat_exe(self, wix_exe):
        """Find heat.exe in the same directory as wix.exe"""
        heat_exe = wix_exe.parent / "heat.exe"
        if heat_exe.exists():
            return heat_exe
        return None
        
    def harvest_files(self, wix_exe):
        """Harvest files from installation directory using heat"""
        self.log("Harvesting files from installation directory...")
        
        heat_output = self.build_dir / "product_fragment.wxs"
        heat_exe = self.find_heat_exe(wix_exe)
        
        if heat_exe:
            # Use heat.exe to harvest files
            self.log(f"Using heat.exe to harvest files")
            cmd = [
                str(heat_exe),
                "dir",
                str(self.install_root),
                "-dr", "INSTALL_ROOT",
                "-cg", "ProductFeature",
                "-gg",
                "-sfrag",
                "-srd",
                "-var", "var.SourceDir",
                "-out", str(heat_output)
            ]
            self.run_command(cmd)
        else:
            # Create a basic fragment manually
            self.log("heat.exe not found, creating basic file fragment...")
            self.create_file_fragment(heat_output)
        
        return heat_output
        
    def create_file_fragment(self, output_file):
        """Create a basic file fragment (simplified version)"""
        # This is a simplified version that creates a minimal component structure
        # In production, heat.exe should be used for complete file harvesting
        
        self.log("WARNING: Creating simplified file fragment. For production builds, ensure heat.exe is available.", "WARN")
        
        # Scan for files in install_root
        files_to_include = []
        for ext in ['*.exe', '*.dll', '*.html', '*.pdf']:
            files_to_include.extend(self.install_root.rglob(ext))
        
        # Create components
        components = []
        component_refs = []
        
        for idx, file_path in enumerate(files_to_include[:50]):  # Limit to avoid huge files
            rel_path = file_path.relative_to(self.install_root)
            file_id = f"File_{idx}"
            comp_id = f"Component_{idx}"
            
            # Sanitize IDs
            file_id = re.sub(r'[^A-Za-z0-9_.]', '_', str(rel_path))
            comp_id = f"Comp_{file_id}"
            
            components.append(f'      <Component Id="{comp_id}" Guid="*">')
            components.append(f'        <File Id="{file_id}" Source="$(var.SourceDir)\\{rel_path}" KeyPath="yes" />')
            components.append(f'      </Component>')
            
            component_refs.append(f'      <ComponentRef Id="{comp_id}" />')
        
        content = f"""<?xml version="1.0" encoding="UTF-8"?>
<Wix xmlns="http://wixtoolset.org/schemas/v4/wxs">
  <Fragment>
    <DirectoryRef Id="INSTALL_ROOT">
{chr(10).join(components)}
    </DirectoryRef>
  </Fragment>
  
  <Fragment>
    <ComponentGroup Id="ProductFeature">
{chr(10).join(component_refs)}
    </ComponentGroup>
  </Fragment>
</Wix>
"""
        
        output_file.write_text(content)
        self.log(f"Created fragment with {len(component_refs)} components")
        
    def create_properties_file(self):
        """Create properties include file"""
        properties_file = self.build_dir / "properties.wxi"
        
        content = """<?xml version="1.0" encoding="UTF-8"?>
<Include>
  <!-- Additional properties can be defined here -->
</Include>
"""
        
        properties_file.write_text(content)
        return properties_file
        
    def build_installer(self, wix_exe):
        """Build the MSI installer using WiX"""
        self.log(f"Building installer for {self.app_name} {self.version}...")
        
        # Create necessary include files
        self.create_license_file()
        variables_file = self.create_wix_variables_file()
        properties_file = self.create_properties_file()
        fragment_file = self.harvest_files(wix_exe)
        
        # Ensure output directory exists
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Build the installer
        # WiX v4+ uses: wix build [options] source.wxs
        cmd = [
            str(wix_exe),
            "build",
            "-arch", self.arch,
            "-out", str(self.output_msi),
            "-ext", "WixToolset.Util.wixext",
            "-d", f"SourceDir={self.install_root}",
            str(self.wix_template),
            str(fragment_file)
        ]
        
        # Add extra sources
        for source in self.wix_extra_sources:
            cmd.append(str(source))
        
        # Add bindpaths for locating files
        cmd.extend(["-bindpath", str(self.install_root)])
        cmd.extend(["-bindpath", str(self.windows_share)])
        cmd.extend(["-bindpath", str(self.build_dir)])
            
        self.log(f"Building MSI: {self.output_msi}")
        if self.verbose:
            self.log(f"Command: {' '.join(cmd)}", "DEBUG")
            
        self.run_command(cmd)
        
        if self.output_msi.exists():
            size_mb = self.output_msi.stat().st_size / (1024 * 1024)
            self.log(f"Successfully created installer: {self.output_msi} ({size_mb:.2f} MB)", "SUCCESS")
            return True
        else:
            self.log("Installer build failed - output file not created", "ERROR")
            return False
            
    def build(self):
        """Main build process"""
        self.log(f"KeePassXC WiX Installer Builder")
        self.log(f"Version: {self.version}")
        self.log(f"Architecture: {self.arch}")
        self.log(f"Build directory: {self.build_dir}")
        self.log(f"Source directory: {self.source_dir}")
        
        # Find WiX
        wix_exe = self.find_wix_exe()
        wix_version = self.get_wix_version(wix_exe)
        self.log(f"Using WiX v{wix_version} at {wix_exe}")
        
        # Verify files
        self.verify_files()
        
        # Build installer
        success = self.build_installer(wix_exe)
        
        if not success:
            sys.exit(1)


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Build KeePassXC Windows installer using WiX Toolset v4+",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument(
        "--build-dir",
        default=os.getcwd(),
        help="Build directory (default: current directory)"
    )
    
    parser.add_argument(
        "--source-dir",
        default=None,
        help="Source directory (default: parent of build dir)"
    )
    
    parser.add_argument(
        "--output-dir",
        default=None,
        help="Output directory for MSI (default: build dir)"
    )
    
    parser.add_argument(
        "--version",
        default=None,
        help="Version string (e.g., 2.7.9). If not specified, reads from CMakeCache.txt"
    )
    
    parser.add_argument(
        "--arch",
        choices=["x64", "x86", "arm64"],
        default="x64",
        help="Target architecture (default: x64)"
    )
    
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    
    args = parser.parse_args()
    
    # Set defaults
    if args.source_dir is None:
        args.source_dir = Path(args.build_dir).parent
        
    if args.output_dir is None:
        args.output_dir = args.build_dir
        
    # Create builder and run
    builder = WixBuilder(args)
    builder.build()


if __name__ == "__main__":
    main()
