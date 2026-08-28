import os
import subprocess
from qt_material import export_theme

# 1. Export light theme 'light_blue.xml'
export_theme(theme='light_blue.xml', qss='style.qss', rcc='temp.qrc')

# 2. Fix the icon urls in style.qss to point to qrc:/icon/
if os.path.exists('style.qss'):
    with open('style.qss', 'r') as f:
        qss_content = f.read()
    
    # Replace all 'icon:/' with 'qrc:/icon/'
    qss_content = qss_content.replace('icon:/', 'qrc:/icon/')
    
    with open('style.qss', 'w') as f:
        f.write(qss_content)
    print("Fixed style.qss icon paths to qrc:/icon/")

# 3. Create a clean resources.qrc file
qrc_lines = [
    '<RCC>',
    '  <qresource prefix="icon">'
]

# Add files from active, primary, and disabled folders
for folder in ['active', 'primary', 'disabled']:
    folder_path = os.path.join('theme', folder)
    if os.path.exists(folder_path):
        for filename in sorted(os.listdir(folder_path)):
            if filename.endswith('.svg'):
                alias = f"{folder}/{filename}"
                filepath = f"theme/{folder}/{filename}"
                qrc_lines.append(f'    <file alias="{alias}">{filepath}</file>')

qrc_lines.extend([
    '  </qresource>',
    '</RCC>'
])

with open('resources.qrc', 'w') as f:
    f.write('\n'.join(qrc_lines) + '\n')
print("Generated resources.qrc")

# 4. Compile resources.qrc to resources.rcc using rcc
try:
    subprocess.run(['rcc', '-binary', '-o', 'resources.rcc', 'resources.qrc'], check=True)
    print("Compiled resources.qrc to resources.rcc successfully.")
except Exception as e:
    print(f"Error compiling RCC: {e}")

# 5. Clean up temporary files
if os.path.exists('temp.qrc'):
    os.remove('temp.qrc')
if os.path.exists('resources.qrc'):
    os.remove('resources.qrc')
