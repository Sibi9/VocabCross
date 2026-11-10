from PIL import Image, ImageDraw, ImageFont
import arabic_reshaper
from bidi.algorithm import get_display

texts = ['رشوة / ارتشاء', 'الترجمة العربية', 'متداول بكثرة في البكالوريا', 'فاسد / يفسد']
fonts = ['assets/fonts/Cairo.ttf', 'assets/fonts/tahoma.ttf', 'assets/fonts/segoeui.ttf']
for fi, fp in enumerate(fonts):
    im = Image.new('RGB', (980, 360), (20, 32, 28))
    dr = ImageDraw.Draw(im)
    for i, s in enumerate(texts):
        shaped = arabic_reshaper.reshape(s)
        disp = get_display(shaped)
        font = ImageFont.truetype(fp, 40 if i < 2 else 24)
        dr.text((20, 20 + i * 85), disp, font=font, fill=(255, 255, 255))
        # also render the non-bidi shaped string on the right for comparison
        dr.text((520, 20 + i * 85), shaped, font=font, fill=(254, 240, 138))
    im.save(f'verification_reports/font_test_{fi}.png')
    print(fi, fp)
