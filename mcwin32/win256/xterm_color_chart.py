#!/usr/bin/python

"""
XTerm Colour Chart

Ian Ward, 2007-2012
This file is in the Public Domain, do with it as you wish.
"""

import sys
from optparse import OptionParser

__version__ = "2.1"

#  Colour charts
#  -------------
#  Anm - colour cube colour where A is a letter between "a" and "f" and
#        n and m are numbers between 0 and 5.  eg. "a00" is the one corner
#        of the cube and "f55" is the opposite corner.  The first coordinate
#        is given as a letter to help distinguish the boundaries between
#        colours in the charts.  In 88-colour mode only values "a" through
#        "d" and 0 through 3 are used.
#  .nn - basic colour where nn is between 00 and 15.
#  +nn - gray colour where nn is between 01 and 24 for 256-colour mode
#        or between 01 and 08 for 88-colour mode.

whale_shape_left = """
e04d04c04b04
e03d03c03b03
e02d02c02b02
e01d01c01b01
e00d00c00b00a00a01a02a03a04a05b05c05d05e05f05f04f03f02f01f00
e10d10c10b10a10a11a12a13a14a15b15c15d15e15f15f14f13f12f11f10
e20d20c20b20a20a21a22a23a24a25b25c25d25e25f25f24f23f22f21f20
e30d30c30b30a30a31a32a33a34a35b35c35d35e35f35f34f33f32f31f30
e40d40c40b40a40a41a42a43a44a45b45c45d45e45f45f44f43f42f41f40
e50d50c50b50a50a51a52a53a54a55b55c55d55e55f55f54f53f52f51f50
                              b54c54d54e54
                              b53c53d53e53
.00.01.02.03.04.05.06.07      b52c52d52e52
.08.09.10.11.12.13.14.15      b51c51d51e51
"""

whale_shape_right = """
d13c13
d12c12
d11c11b11b12b13b14c14d14e14e13e12e11
d21c21b21b22b23b24c24d24e24e23e22e21
d31c31b31b32b33b34c34d34e34e33e32e31
d41c41b41b42b43b44c44d44e44e43e42e41
                  c43d43
                  c42d42
c22c23d23d22
c32c33d33d32


+12+11+10+09+08+07+06+05+04+03+02+01
+13+14+15+16+17+18+19+20+21+22+23+24
"""
# join left and right whales
whale_shape = "\n".join([l.ljust(63)+r for l,r in
    zip(whale_shape_left.split("\n"), whale_shape_right.split("\n"))])

whale_shape_88 = """
c02b02                                    b11b12c12c11
c01b01                                    b21b22c22c21
c00b00a00a01a02a03b03c03d03d02d01d00
c10b10a10a11a12a13b13c13d13d12d11d10      +08+07+06+05+04+03+02+01
c20b20a20a21a22a23b23c23d23d22d21d20
c30b30a30a31a32a33b33c33d33d32d31d30
                  b32c32                  .00.01.02.03.04.05.06.07
                  b31c31                  .08.09.10.11.12.13.14.15

"""

cloud_shape = """
.00.01.02.03.04.05.06.07               c12c13            d13d12
.08.09.10.11.12.13.14.15      d11c11b11b12b13b14c14d14e14e13e12e11
                              d21c21b21b22b23b24c24d24e24e23e22e21
                           e31d31c31b31b32b33b34c34d34e34e33e32
         c22c23d23d22      e41d41c41b41b42b43b44c44d44e44e43e42   +01+24
      d32c32c33d33            d42c42            c43d43            +02+23
                                                                  +03+22
                     c02c03                        d03d02         +04+21
      d01c01      b01b02b03b04      c04d04      e04e03e02e01      +05+20
   e00d00c00b00a00a01a02a03a04a05b05c05d05e05f05f04f03f02f01f00   +06+19
   e10d10c10b10a10a11a12a13a14a15b15c15d15e15f15f14f13f12f11f10   +07+18
   e20d20c20b20a20a21a22a23a24a25b25c25d25e25f25f24f23f22f21f20   +08+17
f30e30d30c30b30a30a31a32a33a34a35b35c35d35e35f35f34f33f32f31      +09+16
f40e40d40c40b40a40a41a42a43a44a45b45c45d45e45f45f44f43f42f41      +10+15
f50e50d50c50b50a50a51a52a53a54a55b55c55d55e55f55f54f53f52f51      +11+14
   e51d51c51b51      b52b53      b54c54d54e54      e53e52         +12+13
      d52c52                        c53d53
"""

cloud_shape_88 = """
                  b11b12c12c11
               c21b21b22c22            b01b02            c02c01
                              c00b00a00a01a02a03b03c03d03d02d01d00
+08+07+06+05+04+03+02+01      c10b10a10a11a12a13b13c13d13d12d11d10
                           d20c20b20a20a21a22a23b23c23d23d22d21
.00.01.02.03.04.05.06.07   d30c30b30a30a31a32a33b33c33d33d32d31
.08.09.10.11.12.13.14.15      c31b31            b32c32
"""

slices = """
a00a01a02a03a04a05   c05c04c03c02c01c00   e00e01e02e03e04e05   +01+24   .00.08
a10a11a12a13a14a15   c15c14c13c12c11c10   e10e11e12e13e14e15   +02+23   .01.09
a20a21a22a23a24a25   c25c24c23c22c21c20   e20e21e22e23e24e25   +03+22   .02.10
a30a31a32a33a34a35   c35c34c33c32c31c30   e30e31e32e33e34e35   +04+21   .03.11
a40a41a42a43a44a45   c45c44c43c42c41c40   e40e41e42e43e44e45   +05+20   .04.12
a50a51a52a53a54a55   c55c54c53c52c51c50   e50e51e52e53e54e55   +06+19   .05.13
b50b51b52b53b54b55   d55d54d53d52d51d50   f50f51f52f53f54f55   +07+18   .06.14
b40b41b42b43b44b45   d45d44d43d42d41d40   f40f41f42f43f44f45   +08+17   .07.15
b30b31b32b33b34b35   d35d34d33d32d31d30   f30f31f32f33f34f35   +09+16
b20b21b22b23b24b25   d25d24d23d22d21d20   f20f21f22f23f24f25   +10+15
b10b11b12b13b14b15   d15d14d13d12d11d10   f10f11f12f13f14f15   +11+14
b00b01b02b03b04b05   d05d04d03d02d01d00   f00f01f02f03f04f05   +12+13
"""


slices_88 = """
a00a01a02a03   c03c02c01c00   +01   .00.08
a10a11a12a13   c13c12c11c10   +02   .01.09
a20a21a22a23   c23c22c21c20   +03   .02.10
a30a31a32a33   c33c32c31c30   +04   .03.11
b30b31b32b33   d33d32d31d30   +05   .04.12
b20b21b22b23   d23d22d21d20   +06   .05.13
b10b11b12b13   d13d12d11d10   +07   .06.14
b00b01b02b03   d03d02d01d00   +08   .07.15
"""


ribbon_left = """
a00a01a02a03a04a05b05c05d05e05f05f04f03f02f01f00e00d00c00b00
a10a11a12a13a14a15b15c15d15e15f15f14f13f12f11f10e10d10c10b10
a20a21a22a23a24a25b25c25d25e25f25f24f23f22f21f20e20d20c20b20
a30a31a32a33a34a35b35c35d35e35f35f34f33f32f31f30e30d30c30b30
a40a41a42a43a44a45b45c45d45e45f45f44f43f42f41f40e40d40c40b40
a50a51a52a53a54a55b55c55d55e55f55f54f53f52f51f50e50d50c50b50

.00.01.02.03.04.05.06.07   +01+02+03+04+05+06+07+08+09+10+11
.08.09.10.11.12.13.14.15
"""

ribbon_right = """
b01c01d01e01e02e03e04d04c04b04b03c03d03d02c02b02
b11c11d11e11e12e13e14d14c14b14b13c13d13d12c12b12
b21c21d21e21e22e23e24d24c24b24b23c23d23d22c22b22
b31c31d31e31e32e33e34d34c34b34b33c33d33d32c32b32
b41c41d41e41e42e43e44d44c44b44b43c43d43d42c42b42
b51c51d51e51e52e53e54d54c54b54b53c53d53d52c52b52

+12+13+14+15+16+17+18+19+20+21+22+23+24

"""

ribbon = "\n".join([l+r for l,r in
    zip(ribbon_left.split("\n"), ribbon_right.split("\n"))])

ribbon_88 = """
a00a01a02a03b03c03d03d02d01d00c00c01c02b02b01b00
a10a11a12a13b13c13d13d12d11d10c10c11c12b12b11b10
a20a21a22a23b23c23d23d22d21d20c20c21c22b22b21b20
a30a31a32a33b33c33d33d32d31d30c30c31c32b32b31b30

.00.01.02.03.04.05.06.07   +01+02+03+04+05+06+07+08
.08.09.10.11.12.13.14.15
"""

cow_shape_left = """
+13+14+15+16+17+18+19+20+21+22+23+24         c01   e01
+12+11+10+09+08+07+06+05+04+03+02+01      b02c02d02e02f02
                                          b03c03d03e03f03f13f23
               d01   b01                  b04c04d04e04f04f14f24
      f01f00e00d00c00b00a00a01a02a03a04a05b05c05d05e05f05f15f25
   f12f11f10e10d10c10b10a10a11a12a13a14a15b15c15d15e15
f32f22f21f20e20d20c20b20a20a21a22a23a24a25b25c25d25e25
f42   f31f30e30d30c30b30a30a31a32a33a34a35b35c35d35e35f35
      f41f40e40d40c40b40a40a41a42a43a44a45b45c45d45e45f45
      f51f50e50d50c50b50a50a51a52a53a54a55b55c55d55e55f55
      f52   e51d51c51b51                  b54c54      f54
      f53   e52d52c52b52                  b53c53      f44
      f43   e53                              d53      f34
      f33   e54                              d54
"""

cow_shape_right = """
   c23d23d22
c32c33d33
c22   d32                  c12d12e12
                           c13d13e13e23
      e11d11c11b11b12b13b14c14d14e14e24
   e22e21d21c21b21b22b23b24c24d24
   e32e31d31c31b31b32b33b34c34d34
   e33   d41c41b41      b44   d44
   e34   d42c42b42      c44   d43
   e44   e42            c43   e43
         e41            b43

.00.01.02.03.04.05.06.07
.08.09.10.11.12.13.14.15
"""
# join left and right cows
cow_shape = "\n".join([l.ljust(66)+r for l,r in
    zip(cow_shape_left.split("\n"), cow_shape_right.split("\n"))])

cow_shape_88 = """
.00.01.02.03.04.05.06.07      b12c12c11
.08.09.10.11.12.13.14.15   b21b22c22
                           b11   c21
+01+02+03+04+05+06+07+08
                           b01c01d01
                           b02c02d02d12
      d00c00b00a00a01a02a03b03c03d03d13
   d11d10c10b10a10a11a12a13b13c13
   d21d20c20b20a20a21a22a23b23c23
   d22   c30b30a30      a33   c33
   d23   c31b31a31      b33   c32
   d33   d31            b32   d32
         d30            a32
"""

charts = {
    88: {
        'cows': cow_shape_88,
        'whales': whale_shape_88,
        'slices': slices_88,
        'ribbon': ribbon_88,
        'clouds': cloud_shape_88,},
    256: {
        'cows': cow_shape,
        'whales': whale_shape,
        'slices': slices,
        'ribbon': ribbon,
        'clouds': cloud_shape,}}

# global settings

basic_start = 0 # first index of basic colours
cube_start = 16 # first index of colour cube
cube_size = 6 # one side of the colour cube
gray_start = cube_size ** 3 + cube_start
colours = 256
# values copied from xterm 256colres.h:
cube_steps = 0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff
gray_steps = (0x08, 0x12, 0x1c, 0x26, 0x30, 0x3a, 0x44, 0x4e, 0x58, 0x62,
    0x6c, 0x76, 0x80, 0x84, 0x94, 0x9e, 0xa8, 0xb2, 0xbc, 0xc6, 0xd0,
    0xda, 0xe4, 0xee)
# values copied from X11/rgb.txt and XTerm-col.ad:
basic_colours = ((0,0,0), (205, 0, 0), (0, 205, 0), (205, 205, 0),
    (0, 0, 238), (205, 0, 205), (0, 205, 205), (229, 229, 229),
    (127, 127, 127), (255, 0, 0), (0, 255, 0), (255, 255, 0),
    (0x5c, 0x5c, 0xff), (255, 0, 255), (0, 255, 255), (255, 255, 255))

def set_88_colour_mode():
    """Switch to 88-colour mode."""
    global cube_size, gray_start, colours, cube_steps, gray_steps
    cube_size = 4
    gray_start = cube_size ** 3 + cube_start
    colours = 88
    # values copied from xterm 88colres.h:
    cube_steps = 0x00, 0x8b, 0xcd, 0xff
    gray_steps = 0x2e, 0x5c, 0x73, 0x8b, 0xa2, 0xb9, 0xd0, 0xe7


def error(e):
    """Report an error to the user."""
    sys.stderr.write(e+"\n")

def cube_vals(n):
    """Return the cube coordinates for colour-number n."""
    assert n>=cube_start and n<gray_start
    val = n-cube_start
    c = val % cube_size
    val = val / cube_size
    b = val % cube_size
    a = val / cube_size
    return a, b, c

def n_to_rgb(n):
    """Return the red, green and blue components of colour-number n.
    Components are between 0 and 255."""
    if n<cube_start:
        return basic_colours[n-basic_start]
    if n<gray_start:
        return [cube_steps[v] for v in cube_vals(n)]
    return (gray_steps[n-gray_start],) * 3

def n_to_gray(n):
    """Return an approximate desaturated value for colour-number n.
    Value is between 0.0 and 255.0."""
    r, g, b = n_to_rgb(n)
    return 0.3*r + 0.59*g + 0.11*b

def n_to_prt(n):
    """Convert a colour number to the format used in the colour charts."""
    if n >= gray_start:
        return "+%02d" % (n-gray_start+1)
    elif n >= cube_start:
        a, b, c = cube_vals(n)
        return "%s%s%s" % (chr(ord('a')+a), chr(ord('0')+b), chr(ord('0')+c))
    else:
        return ".%02d not found" % (n-basic_start)

def prt_to_n(prt):
    """Convert a colour chart cell to a colour number."""
    assert len(prt)==3
    if prt == '   ':
        n = -1
    elif prt[0] == '.':
        val = int(prt[1:])
        assert val>=0 and val<cube_start
        n = basic_start + val
    elif prt[0] == '+':
        val = int(prt[1:])-1
        assert val>=0 and val<colours-gray_start, prt
        n = gray_start + val
    else:
        a = ord(prt[0])-ord('a')
        assert a>=0 and a<cube_size, prt
        b = ord(prt[1])-ord('0')
        assert b>=0 and b<cube_size, prt
        c = ord(prt[2])-ord('0')
        assert c>=0 and c<cube_size, prt
        n = cube_start + (a*cube_size + b)*cube_size + c
    return n

def urwidify(num):
    """Return an urwid palette colour name for num"""
    if num < cube_start:
        return 'h%d' % num
    if num < gray_start:
        num -= cube_start
        b, num = num % cube_size, num // cube_size
        g, num = num % cube_size, num // cube_size
        r = num % cube_size
        return '#%x%x%x' % (int_scale(cube_steps[r], 256, 16),
            int_scale(cube_steps[g], 256, 16),
            int_scale(cube_steps[b], 256, 16))
    return 'g%d' % (int_scale(gray_steps[num - gray_start], 256, 101))

def int_scale(val, val_range, out_range):
    num = int(val * (out_range-1) * 2 + (val_range-1))
    dem = ((val_range-1) * 2)
    return num // dem

def distance(n1, n2):
    """Calculate the distance between colours in the colour cube.
    Distance is absolute cube coordinates, not actual colour distance.
    Return -1 if one of the colours is not part of the colour cube."""
    if n1<cube_start or n1>=gray_start:
        return -1
    if n2<cube_start or n2>=gray_start:
        return -1
    a1, b1, c1 = cube_vals(n1)
    a2, b2, c2 = cube_vals(n2)
    return abs(a1-a2)+abs(b1-b2)+abs(c1-c2)

def parse_chart(chart):
    """Parse a colour chart passed in as a string."""
    chart = chart.rstrip()
    found = set()

    oall = [] # the complete chart output
    for ln in chart.split('\n'):
        oln = [] # the current line of output
        ln = ln.rstrip()
        if not oall and not ln:
            # remove blank lines from top of chart
            continue

        for loff in range(0, len(ln), 3):
            prt = ln[loff:loff+3]
            if not prt:
                continue
            n = prt_to_n(prt)
            if n>=0 and n in found:
                error("duplicate entry %s found" % prt)
            found.add(n)
            if oall and len(oall[-1])>len(oln): # compare distance above
                nabove = oall[-1][len(oln)]
                if distance(nabove, n)>1:
                    error("entry %s found above %s" % (n_to_prt(nabove), prt))
            if oln: # compare distance to left
                nleft = oln[-1]
                if distance(nleft, n)>1:
                    error("entry %s found left of %s" % (n_to_prt(nleft), prt))
            oln.append(n)
        oall.append(oln)

    # make sure all colours were included in the chart
    for n in range(colours):
        if n in found:
            continue
        error("entry %s not found" % n_to_prt(n))

    return oall


def draw_chart(chart, origin, angle, hexadecimal, decimal, urwidmal, cell_cols,
        cell_rows):
    """Draw a colour chart on the screen.

    chart -- chart data parsed by parse_chart()
    origin -- 0..7 origin of colour cube
    angle -- 0..5 rotation angle of colour cube
    hexadecimal -- if True display hex palette numbers on the chart
    decimal -- if True display decimal palette numbers on the chart
    urwidmal -- if True display urwid palette colour on the chart
    cell_cols -- number of screen columns per cell
    cell_rows -- number of screen rows per cell
    """
    amap = [(0,1,2), (1,2,0), (2,0,1), (0,2,1), (1,0,2), (2,1,0)][angle]
    omap = [(1,1,1), (1,1,-1), (1,-1,-1), (1,-1,1),
        (-1,-1,1), (-1,-1,-1), (-1,1,-1), (-1,1,1)][origin]

    if hexadecimal and cell_cols < 2:
        cell_cols = 2
    elif decimal and cell_cols < 3:
        cell_cols = 3
    elif urwidmal and cell_cols < 4:
        cell_cols = 4
    cell_pad = " "*cell_cols

    def transform_block(n, row):
        v = cube_vals(n)
        v = [(int(om/2) + om * n) % cube_size for n, om in zip(v, omap)]
        r, g, b = v[amap[0]], v[amap[1]], v[amap[2]]
        vtrans = (r*cube_size + g)*cube_size + b + cube_start
        return block(vtrans, row)

    def block(n, row):
        if not any((hexadecimal, decimal, urwidmal)) or row!=cell_rows-1:
            return "\x1b[48;5;%dm%s" % (n, cell_pad)
        y = n_to_gray(n)
        if y>0x30:
            # use black text
            if hexadecimal:
                return "\x1b[48;5;%d;30m%02x%s" % (n, n, cell_pad[2:])
            elif decimal:
                return "\x1b[48;5;%d;30m%03d%s" % (n, n, cell_pad[3:])
            elif urwidmal:
                return "\x1b[48;5;%d;30m%4s%s" % (n, urwidify(n), cell_pad[4:])
        # else use gray text
        if hexadecimal:
            return "\x1b[48;5;%d;37m%02x%s" % (n, n, cell_pad[2:])
        elif decimal:
            return "\x1b[48;5;%d;37m%03d%s" % (n, n, cell_pad[3:])
        elif urwidmal:
            return "\x1b[48;5;%d;37m%4s%s" % (n, urwidify(n), cell_pad[4:])

    def blank():
        return "\x1b[0m%s" % (cell_pad,)

    for ln in chart:
        for row in range(cell_rows):
            out = []
            for n in ln:
                if n<0:
                    out.append(blank())
                elif n<cube_start:
                    out.append(block(n, row))
                elif n<gray_start:
                    out.append(transform_block(n, row))
                else:
                    out.append(block(n, row))
            print "".join(out) + "\x1b[0m"

def reset_palette():
    """Reset the terminal palette."""
    reset = ["%d;rgb:%02x/%02x/%02x" % ((n,) + tuple(n_to_rgb(n)))
        for n in range(colours)]
    sys.stdout.write("\x1b]4;"+";".join(reset)+"\x1b\\")

def main():
    parser = OptionParser(usage="%prog [options] [chart names]",
        version="%prog "+__version__)
    parser.add_option("-8", "--88-colours", action="store_true",
        dest="colours_88", default=False,
        help="use 88-colour mode [default: 256-colour mode]")
    parser.add_option("-l", "--list-charts", action="store_true",
        dest="list_charts", default=False,
        help="list available charts")
    parser.add_option("-o", "--origin", dest="origin", type="int",
        default=0, metavar="NUM",
        help="set the origin of the colour cube: 0-7 [default: %default]")
    parser.add_option("-a", "--angle", dest="angle", type="int",
        default=0, metavar="NUM",
        help="set the angle of the colour cube: 0-5 [default: %default]")
    parser.add_option("-n", "--numbers", "--hex", action="store_true",
        dest="hexadecimal", default=False,
        help="display hex colour numbers on chart")
    parser.add_option("-d", "--decimal", action="store_true",
        dest="decimal", default=False,
        help="display decimal colour numbers on chart")
    parser.add_option("-u", "--urwid", action="store_true",
        dest="urwidmal", default=False,
        help="display urwid palette colour on chart")
    parser.add_option("-x", "--cell-columns", dest="columns", type="int",
        default=2, metavar="COLS",
        help="set the number of columns for drawing each colour cell "
        "[default: %default]")
    parser.add_option("-y", "--cell-rows", dest="rows", type="int",
        default=1, metavar="ROWS",
        help="set the number of rows for drawing each colour cell "
        "[default: %default]")
    parser.add_option("-r", "--reset-palette", action="store_true",
        dest="reset_palette", default=False,
        help="reset the colour palette before displaying chart, "
        "this option may be used to switch between 88 and 256-colour "
        "modes in xterm")

    options, args = parser.parse_args()
    if options.origin<0 or options.origin>7:
        error("Invalid origin value specified!")
        sys.exit(2)
    if options.angle<0 or options.angle>5:
        error("Invalid angle value specified!")
        sys.exit(2)
    if options.columns < 1:
        error("Invalid number of columns specified!")
    if options.rows < 1:
        error("Invalid number of rows specified!")
    if options.colours_88:
        set_88_colour_mode()
    if options.list_charts:
        print "Charts available in %d-colour mode:" % colours
        for cname in charts[colours].keys():
            print "  "+cname
        sys.exit(0)
    if options.reset_palette:
        reset_palette()

    if not args:
        args = ["whales"] # default chart
    first = True
    for cname in args:
        if not first:
            print
        first = False
        if cname not in charts[colours]:
            error("Chart %r not found!" % cname)
            continue
        chart = parse_chart(charts[colours][cname])
        draw_chart(chart, options.origin, options.angle, options.hexadecimal,
            options.decimal, options.urwidmal, options.columns, options.rows)



if __name__ == '__main__':
    main()

