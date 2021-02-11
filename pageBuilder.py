import os
import re as regex

SOURCE_FILETYPE = '.html'
TARGET_FILETYPE = '.h'

SCRIPT_SRC_REGEX = '<script\s*src\s*=\s*"([a-zA-Z0-9.-_]*)"\s*>\s*<\/script>'
LINK_SRC_REGEX = '<\s*link\s*rel\s*=\s*"([a-zA-Z0-9.-_]*)"\s*href\s*=\s*"([a-zA-Z0-9.-_]*)"\s*>'

RSTRING_DELIM = '&'

# includes the '.' in the return value
def fileType(file:str) -> str:
    return file[file.rfind('.'):].lower()

def genericEmbed(file:str, tag:str) -> str:
    outData = ""
    try:
        with open(file, 'r') as jsF:
            for line in jsF:
                outData += line
    except FileNotFoundError as f:
        print(f"Wile trying to embed {file}, got file not found error.")
        raise f

    return f"<{tag}>\n{outData}\n</{tag}>"

def embedJavascript(jsFile:str) -> str:
    return genericEmbed(jsFile, "script")

def embedCSS(cssFile:str) -> str:
    return genericEmbed(cssFile, "style")

# not sure if this is necessary:
def addEscapes(line:str) -> str:
    return line

def translatePage(sourceFile:str) -> bool:
    assert(fileType(sourceFile) == SOURCE_FILETYPE)

    # name translation: test.html -> test.h
    fileRoot = sourceFile[:len(sourceFile) - len(SOURCE_FILETYPE)]
    print(f"Processing '{fileRoot}', '{SOURCE_FILETYPE}' -> '{TARGET_FILETYPE}'")
    targetFile = f'{fileRoot}{TARGET_FILETYPE}'

    # open the file
    with open(sourceFile, 'r') as sf:
        with open(targetFile, 'w') as tf:
            DELIM_CHAR = RSTRING_DELIM if RSTRING_DELIM is not None else ""
            print(f'const char {fileRoot}_html[] PROGMEM = R"{DELIM_CHAR}(', file=tf)
            RSTRING_CLOSE = f'){DELIM_CHAR}"'
            try:
                for line in sf:
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
                    
                    if RSTRING_CLOSE in outLine:
                        # if this is a common problem, we can programatically retry alternate delimeters
                        raise RuntimeError("RString close found in outline, need alternate delim")

                    print(outLine, file=tf)
            finally:
                print(f'{RSTRING_CLOSE};', file=tf)


if __name__ == "__main__":
    pages = [f for f in os.listdir() if fileType(f) == SOURCE_FILETYPE]

    print(f"Pages found: {pages}")

    for page in pages:
        translatePage(page)