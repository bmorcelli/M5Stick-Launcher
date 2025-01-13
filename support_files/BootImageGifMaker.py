
from PIL import Image, ImageDraw, ImageFont
import random

# Configurações
# WIDTH, HEIGHT = 440, 154
WIDTH, HEIGHT = 440, 354
FGCOLOR = (0, 255, 0)  # Verde
BGCOLOR = (0, 0, 0)    # Preto
even_color = (25, 115, 25)  # Verde escuro
odd_color = (20, 98, 20)    # Verde mais escuro
LW, LH = 6, 10  # Largura e altura de caracteres
frames = []

# Fontes
font_small = ImageFont.load_default()
font_large = ImageFont.truetype("C:\\Windows\\Fonts\\OCRAEXT.TTF",82)
font_medium = ImageFont.truetype("C:\\Windows\\Fonts\\OCRAEXT.TTF",16)

# Função principal para atualizar frames
def update_frame_with_border(img, do_all=False, var_name=0):
    draw = ImageDraw.Draw(img)
    launcher_text = "Launcher"
    name_options = ["@Pirata", "u/bmorcelli", "gh/bmorcelli"]
    name = name_options[var_name]

    x, y = 10, 10

    while y < HEIGHT - 12:
        cor = random.randint(0, 10)
        show = random.randint(0, 39)

        if show == 0 or do_all:
            if cor == 10:
                txt = " "
                color = BGCOLOR
            elif cor % 2 == 0:
                color = even_color
                txt = str(cor)
            else:
                color = odd_color
                txt = str(cor)

            if x >= WIDTH - 10:
                x = 10
                y += LH
            elif x < 10:
                x = 10
            if y >= HEIGHT - 12:
                break

            if y > HEIGHT - 25 and x >= WIDTH - ( LW * 20) and do_all:
                
                x += LW * 25
            else:
                draw.rectangle((x, y, x + LW - 1, y + LH - 1), fill=BGCOLOR)
                draw.text((x, y), txt, fill=color, font=font_small)
                x += LW
        else:
            if y > HEIGHT - 25 and x >= WIDTH - (LW * 20):
                x += LW * 13
            else:
                x += LW

            if x >= WIDTH - 10:
                x = 10
                y += LH

    width=3

    t_width = draw.textlength(name,font=font_medium)
    draw.rectangle((WIDTH - (LW * 20), HEIGHT - 25, WIDTH, HEIGHT - 25 + LH*2 - 1), fill=BGCOLOR)
    draw.text((WIDTH - (LW * 10) - t_width/2, HEIGHT - 25), name, fill=FGCOLOR, font=font_medium)

    t_width = draw.textlength(launcher_text,font=font_large)
    draw.text((WIDTH // 2 - t_width/2, HEIGHT // 2 - 5*LH), launcher_text, fill=FGCOLOR, font=font_large, stroke_width=3, stroke_fill=(0,50,0))

    draw.rounded_rectangle(
        (width, width, WIDTH - width, HEIGHT - width),
        radius=5,
        outline=FGCOLOR,
        width=width,
    )
    return img

# Gera o primeiro frame com fundo completo e borda
img = Image.new("RGB", (WIDTH, HEIGHT), BGCOLOR)
img = update_frame_with_border(img, do_all=True,var_name=1)
frames = [img.copy()]

# Gera frames subsequentes utilizando o frame anterior como base
for i in range(49):  # Total de 50 frames
    if i < 25:
        img = update_frame_with_border(img, do_all=False,var_name=1)
    else:
        img = update_frame_with_border(img, do_all=False,var_name=0)
    frames.append(img.copy())

# Salva o GIF com a borda arredondada
frames[0].save(
    "animation_with_border.gif",
    save_all=True,
    append_images=frames[1:],
    optimize=False,
    duration=100,  # Duração por frame em ms
    loop=0
)
