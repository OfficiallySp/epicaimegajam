"""Assemble gpt-image-2 wall flats; run from any directory with Pillow installed."""
from pathlib import Path
from PIL import Image, ImageOps, ImageDraw, ImageFont
ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'assets/src/backdrops'
W,H=512,640

def panel(path,reel):
    im=ImageOps.fit(Image.open(path).convert('RGB'),(W,H),method=Image.Resampling.LANCZOS)
    colors={1:(30,104,116),2:(112,54,44),3:(74,30,36)}
    template=Image.new('RGB',(W,H),colors[reel]); d=ImageDraw.Draw(template)
    if reel==1:
        d.rectangle((0,460,511,639),fill=(64,40,24))
        d.rectangle((0,460,511,470),fill=(120,80,44))
        for x in (28,268): d.rectangle((x,494,x+216,614),outline=(104,68,38),width=6)
        d.rectangle((0,28,511,36),fill=(196,160,88))
    elif reel==2:
        d.rectangle((0,570,511,639),fill=(34,22,27))
        d.rectangle((0,0,511,15),fill=(58,34,34))
    else:
        d.rectangle((0,596,511,603),fill=(230,60,50))
        d.rectangle((0,604,511,639),fill=(40,16,22))
        d.rectangle((0,0,511,15),fill=(28,10,14))
    mask=Image.new('L',(W,H)); p=mask.load()
    for y in range(H):
        band= (y>=460 or y<=36) if reel==1 else (y>=570 or y<16) if reel==2 else (y>=596 or y<16)
        for x in range(W):
            edge=min(x,W-1-x)
            p[x,y]=255 if band or edge<16 else max(0,int(255*(48-edge)/32))
    return Image.composite(template,im,mask)

def main():
    # Load all sources before overwriting any atlas.
    reels=[[panel(OUT/'raw'/f'R{r}_{i}.png',r) for i in range(1,5)] for r in range(1,4)]
    contact=Image.new('RGB',(4096,6*H),(24,24,24))
    for r,panels in enumerate(reels,1):
        atlas=Image.new('RGB',(2048,H))
        for i,p in enumerate(panels): atlas.paste(p,(i*W,0))
        atlas.save(OUT/f'T_Backdrop_R{r}.png'); contact.paste(atlas,(0,(r-1)*H))
        for i,k in enumerate((2,0,3,1,3,0,1,2)): contact.paste(panels[k],(i*W,(r+2)*H))
        for a in panels:
            for b in panels: assert a.crop((511,0,512,H)).tobytes()==b.crop((0,0,1,H)).tobytes()
    contact.save(OUT/'contact.png')
    key=ROOT/'assets/src/keyart'; key.mkdir(parents=True,exist_ok=True)
    raw=Image.open(OUT/'raw/keyart.png'); raw.save(key/'keyart_raw.png')
    cover=ImageOps.fit(raw.convert('RGB'),(630,500),method=Image.Resampling.LANCZOS)
    fonts=list(ROOT.rglob('*Bebas*.ttf'))
    if fonts:
        d=ImageDraw.Draw(cover); f=ImageFont.truetype(str(fonts[0]),72)
        d.text((315,478),'FIX IT IN POST',font=f,anchor='ms',fill=(255,235,186),stroke_width=3,stroke_fill=(30,20,24))
    cover.save(ROOT/'devlog/img/cover-630x500-ai.png')
    print('Wrote three RGB 2048x640 atlases, contact and 630x500 cover; all 48 directed edge pairs match.')
if __name__=='__main__': main()
