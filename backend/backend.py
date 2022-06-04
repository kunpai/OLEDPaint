import json
from random import randint
from xml.dom.minidom import Element
from flask import Flask, request
from pyparsing import col
import svgwrite
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from svglib.svglib import svg2rlg
from reportlab.graphics import renderPM


def chunkArray(array, size):
    result = []
    for i in range(0, len(array), size):
        result.append(array[i: i+size])
    return result

def slice_per(source, step):
    return [source[i::step] for i in range(step)]


app = Flask(__name__)
dwg = svgwrite.Drawing('test.svg', profile='tiny', size=("128px","128px"))
color = svgwrite.rgb(255,255,255)
#lookuptable = {"65535": svgwrite.rgb(255,255,255), "0": svgwrite.rgb(0,0,0), "63488": svgwrite.rgb(255,0,0), "2016": svgwrite.rgb(0,255,0), "31": svgwrite.rgb(0,0,255), "2047": svgwrite.rgb(0,255,255), "63519": svgwrite.rgb(255,0,255), "65504": svgwrite.rgb(255,255,0)}
dwg.add(dwg.rect(insert=(0, 0), size=('100%', '100%'), rx=None, ry=None, fill='rgb(0,0,0)'))
dwg.save()

lookuptable = {"d": svgwrite.rgb(255,255,255), "h": svgwrite.rgb(0,0,0), "a": svgwrite.rgb(255,0,0), "f": svgwrite.rgb(0,255,0), "g": svgwrite.rgb(0,0,255), "e": svgwrite.rgb(0,255,255), "b": svgwrite.rgb(255,0,255), "c": svgwrite.rgb(255,255,0)}



bitmap = ""
c = 0

# def count():
#     global c
#     c += 1
#     if (c == 64):
#         print("Yo")
#     print(c)

# for i in range(64):
#     count()

@app.route('/draw', methods = ['POST'])
def draw():
    returnJSON = request.get_json()
    print("received")
    print(returnJSON)
    input = returnJSON['map']
    global bitmap
    bitmap = bitmap + input
    global c
    c += 1
    if (c >= 16):
        rows, cols = (c, 128)
        rowsize = str(cols)+"px"
        colsize = str(cols)+ "px"
        dwg = svgwrite.Drawing('bimage.svg', profile='tiny', size=(rowsize,colsize))
        dwg.add(dwg.rect(insert=(0, 0), size=('100%', '100%'), rx=None, ry=None, fill='rgb(0,0,0)'))
        dwg.save()

        result = slice_per(bitmap, 128)
        print(len(result))
        newresult = []
        for i in range(len(result)):
            newresult.append(slice_per(result[i],128))

        for i in range(0,128):
            for j in range(0, 128):
                dwg.add(dwg.rect(insert=(j, i), size=('1px', '1px'), rx=None, ry=None, fill=lookuptable[newresult[i][j]]))

        dwg.save() 
        print("image gen")
        drawing = svg2rlg('bimage.svg')
        renderPM.drawToFile(drawing, 'test.png', fmt='PNG')
        # svg2png(url="test.svg",  write_to="test.png")
        img = mpimg.imread('test.png')
        imgplot = plt.imshow(img)
        plt.show()
        c = 0
        bitmap = ""
        result = []
        
    print(c)

    return "0"
    
    

    #     rows, cols = (int((len(input)**(1/2))), int((len(input)**(1/2))))
    #     imgsize = str(rows)+"px"
    #     dwg = svgwrite.Drawing('bimage.svg', profile='tiny', size=(imgsize,imgsize))
    #     dwg.add(dwg.rect(insert=(0, 0), size=('100%', '100%'), rx=None, ry=None, fill='rgb(0,0,0)'))
    #     dwg.save()

    #     result = chunkArray(input, rows)

    #     for i in range(len(result)):
    #         for j in range(len(result)):
    #             dwg.add(dwg.rect(insert=(i, j), size=('1px', '1px'), rx=None, ry=None, fill=lookuptable[result[i][j]]))

    #     dwg.save() 
    #     print("image gen")
    #     drawing = svg2rlg('bimage.svg')
    #     renderPM.drawToFile(drawing, 'test.png', fmt='PNG')
    #     # svg2png(url="test.svg",  write_to="test.png")
    #     img = mpimg.imread('test.png')
    #     imgplot = plt.imshow(img)
    #     plt.show()
    # except Exception as KeyError:
    #     pass

if __name__ == '__main__':
   app.run(debug=True, host='0.0.0.0', port=80)