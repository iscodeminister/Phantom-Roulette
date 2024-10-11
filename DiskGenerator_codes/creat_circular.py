from PIL import Image, ImageDraw,ImageOps
import math
from math import sqrt
from reportlab.lib.pagesizes import A4,A5

def create_circular_single_image(img_path, radius_cm,dpi, background_color=(255, 255, 255),scal=1,height=1,flipl=False):
    img = Image.open(img_path)
    radius= int(radius_cm * dpi / 2.54)
    s1=(radius*2)/max(img.width,img.height)
    img=img.resize((int(img.width*s1),int(img.height*s1)))
    size = (radius * 2, radius * 2)
    circular_image = Image.new('RGBA', size, background_color)
    draw = ImageDraw.Draw(circular_image)
    draw.ellipse([0, 0, size[0]-1, size[1]-1], outline='black')
    #draw center small circle with 1cm phi
    draw.ellipse([radius-0.25*dpi/2.54, radius-0.25*dpi/2.54, radius+0.25*dpi/2.54, radius+0.25*dpi/2.54], fill='black')
    img = img.resize((int(img.width*scal),int( img.height*scal)), Image.LANCZOS)
    x = int(radius - img.width / 2)
    if img.mode != 'RGBA':
        img = img.convert('RGBA')
    circular_image.paste(img, (x,int(100*height)), img)
    return circular_image

def create_circular_animation(gif_path, radius_cm,dpi, background_color=(255, 255, 255),scal=1,height=1,flipl=False):
    gif = Image.open(gif_path)
    n_frames = gif.n_frames
    radius= int(radius_cm * dpi / 2.54)
    size = (radius * 2, radius * 2)
    circular_image = Image.new('RGBA', size, background_color)
    draw = ImageDraw.Draw(circular_image)
    draw.ellipse([0, 0, size[0]-1, size[1]-1], outline='black')
    #draw center small circle with 1cm phi
    draw.ellipse([radius-0.25*dpi/2.54, radius-0.25*dpi/2.54, radius+0.25*dpi/2.54, radius+0.25*dpi/2.54], fill='black')
    angle_step = 360 / n_frames


    for i in range(n_frames):
        gif.seek(i)
        frame = gif.copy().convert('RGBA')
        if flipl:
            frame = ImageOps.flip(frame)
        frame=padding_frame(frame)
        frame_size = int(radius * 0.2 * (2**0.5) * scal)
        frame = frame.resize((frame_size, frame_size), Image.LANCZOS)
        angle = math.radians(i * angle_step)
        frame = frame.rotate(90 - math.degrees(angle))
        x = int(radius + (radius * 0.8 * height) * math.cos(angle) - frame_size / 2)
        y = int(radius + (radius * 0.8 * height) * math.sin(angle) - frame_size / 2)
        circular_image.paste(frame, (x, y), frame)
    
    return circular_image

def padding_frame(frame):
    w=frame.width
    h=frame.height
    new_size = int(sqrt(w**2 + h**2))
    h_extent = (new_size-w)//2
    v_extent = (new_size-h)//2
    padded_im = ImageOps.expand(frame, border=(h_extent, v_extent), fill=(0,0,0,0))
    return padded_im

def combine2page(circular_image,dpi,page_size):
    if page_size == 'A4':
        page_width_inch, page_height_inch = A4[0]/72, A4[1]/72
    elif page_size == 'A5':
        page_width_inch, page_height_inch = A5[0]/72, A5[1]/72
    page_width_px = int(page_width_inch * dpi)
    page_height_px = int(page_height_inch * dpi)
    page_image = Image.new('RGB', (page_width_px, page_height_px), (255, 255, 255))
    paste_x = (page_width_px - circular_image.width) // 2
    paste_y = (page_height_px - circular_image.height) // 2
    page_image.paste(circular_image, (paste_x, paste_y), circular_image)

    return page_image

if __name__ == "__main__":
    create_circular_single_image('光電教具\_DSC3020.jpg', 6,300,(255, 255, 255),0.2,2,False).show()
