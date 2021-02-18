from datetime import datetime
import os
import re as regex

SOURCE_FILETYPE = '.html'
TARGET_FILETYPE = '.h'

SCRIPT_SRC_REGEX = '<script\s*src\s*=\s*"([a-zA-Z0-9.-_]*)"\s*>\s*<\/script>'
LINK_SRC_REGEX = '<\s*link\s*rel\s*=\s*"([a-zA-Z0-9.-_]*)"\s*href\s*=\s*"([a-zA-Z0-9.-_]*)"\s*>'

RSTRING_DELIM = '&'

EXCLUDED_FILES = ["hostPort.js"]

# includes the '.' in the return value
def fileType(file:str) -> str:
    return file[file.rfind('.'):].lower()

def genericEmbed(filePath:str, tag:str) -> str:
    if filePath in EXCLUDED_FILES:
        print(f"Skipping file {filePath} reason: in excluded list.")
        return ""
    outData = ""
    try:
        with open(filePath, 'r') as jsF:
            for line in jsF:
                outData += line
    except FileNotFoundError as f:
        print(f"Wile trying to embed {filePath}, got file not found error.")
        raise f

    return f"<{tag}>\n{outData}\n</{tag}>"

def embedJavascript(jsFile:str) -> str:
    return genericEmbed(jsFile, "script")

def embedCSS(cssFile:str) -> str:
    return genericEmbed(cssFile, "style")

# not sure if this is necessary:
def addEscapes(line:str) -> str:
    return line

def translatePage(sourceFilePath:str) -> bool:
    assert(fileType(sourceFilePath) == SOURCE_FILETYPE)

    # name translation: test.html -> test.h
    fileRoot = sourceFilePath[:len(sourceFilePath) - len(SOURCE_FILETYPE)]
    print(f"Processing '{fileRoot}', '{SOURCE_FILETYPE}' -> '{TARGET_FILETYPE}'")
    targetFilePath = f'{fileRoot}{TARGET_FILETYPE}'

    # open the file
    with open(sourceFilePath, 'r') as sourceFile:
        with open(targetFilePath, 'w') as targetFile:
            DELIM_CHAR = RSTRING_DELIM if RSTRING_DELIM is not None else ""
            print(f'const char {fileRoot}_html[] PROGMEM = R"{DELIM_CHAR}(', file=targetFile)
            RSTRING_CLOSE = f'){DELIM_CHAR}"'
            try:
                for line in sourceFile:
                    line = line.strip() # remove whitespace
                    x = regex.search(SCRIPT_SRC_REGEX, line)
                    y = regex.search(LINK_SRC_REGEX, line)

                    if x is not None:
                        print(f"Found embedable script: {x.group(1)}")
                        outLine = addEscapes(embedJavascript(x.group(1)))
                    elif y is not None:
                        assert(y.group(1) == 'stylesheet')
                        print(f"Found embedable style: {y.group(2)}")
                        outLine = addEscapes(embedCSS(y.group(2)))
                    else:
                        outLine = line

                    if(outLine == ""):
                        outLine = line
                    
                    #check if the ending of the rawstring is in the line
                    if RSTRING_CLOSE in outLine:
                        # if this is a common problem, we can programatically retry alternate delimeters
                        print("Error: the rawstring delimiter was found inside the raw string.")
                        print("Select a new delimiter and try re-running the script.")
                        raise RuntimeError("RString close found in outline, need alternate delimiter")

                    print(outLine, file=targetFile)
            finally:
                print(f'{RSTRING_CLOSE};', file=targetFile)


if __name__ == "__main__":
    now = datetime.now()
    print(now.strftime("%d/%m/%Y %H:%M:%S"))

    pages = [f for f in os.listdir() if fileType(f) == SOURCE_FILETYPE]

    if(len(pages) == 0):
        print("No pages found.")
        exit(0)

    print(f"Pages found: {pages}")

    for page in pages:
        translatePage(page)