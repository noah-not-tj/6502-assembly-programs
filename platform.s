define plat0 $00
define plat1 $01
define temp0 $02
define temp1 $03

define time $04

define rand $fe

define playerY $0a
define maxY $0b

define playerColor $0c

define doublejump $0d


;init time
lda #$00
sta time
sta lastPress
sta lastkey

; init plat 
lda #$ff
sta plat0
lda #$ff
sta plat1

lda #$00
sta doublejump

; init playerY
lda #$80
sta maxY
sta playerY
lda #$04 
sta playerColor

lda #$02
ldx #$20
init_draw_plat:
  dex
  sta $0520,x 
  sta $0540,x
  sta $0560,x
  sta $0580,x
  sta $05a0,x
  sta $05c0,x
  sta $05e0,x
  cpx #$00
  bne init_draw_plat


jmp end

load_temp:
  lda plat0 
  sta temp0
  lda plat1 
  sta temp1
  rts

draw_platform:
  jsr load_temp
  ldx #$00
  
  platLoop:
      lsr temp1    ; input = 0
      ror temp0    ; input = carry
  
      bcc hole
    
  plat:
    lda #$05         ;green
    sta $04a0,x
    lda #$02
      sta $04c0,x
      sta $04e0,x
      sta $0500,x
    inx
    lda #$05         
    sta $04a0,x
    lda #$02
      sta $04c0,x
      sta $04e0,x
      sta $0500,x
    
    jmp nextbit

  hole:
    lda #$00
    sta $04a0,x
    sta $04c0,x
    sta $04e0,x
    sta $0500,x
    inx
    lda #$00
    sta $04a0,x
    sta $04c0,x
    sta $04e0,x
    sta $0500,x
  nextbit:
    inx
    cpx #$20 
    bne platLoop
    rts
  
update_platform:
  jsr random_num
  ror plat1
  ror plat0
  rts


random_num:
  lda rand
  cmp #$80
  rol
  rts
  


delay:
  ldy #$01
  del2:
  ldx #$ff
  del1:
    lda #$00
    sta $0201
    nop
    dex
    bne del1
  dey
  bne del2
  rts

update_player:
  lda playerY
  cmp maxY
  bne gravity
  

  lda $04a8
  cmp #$00
  bne skip_ameover
  jmp gameover
  skip_ameover:

  lda #$00
  sta doublejump

  rts

  
  gravity:
  clc
  adc #$20
  sta playerY
  
 
  rts

  
draw_player:

  ldx playerY
  cpx #$00          ; if already at top don't clear top
  beq draw_p
  
  ldx #$00      
  lda #$00     
  clear_top:
    sta $0408,x    ; clear pixel
    tay               ; x: num     y: a    a: a
    txa               ; x: num     y: a    a: x
    clc
    adc #$20       
    tax
    tya
    cpx playerY    
    bne clear_top
    
  draw_p:  
    lda playerColor
    ldx playerY
    sta $0408,x

  cpx maxY            ; if at the bottom don't clear bottom
  beq player_done
  
  lda #$00
  ldx playerY                ; initial counter is playerY
  clear_bottom:
    tay
    txa
    clc
    adc #$20
    tax
    tya
    sta $0408,x
    cpx maxY
    bne clear_bottom
  
  
  player_done:
    rts
 
readkey:
  lda $ff
  cmp #$77
  beq jump

keydone:
  rts
  
jump:
  lda playerY
  cmp #$00
  beq keydone

  cmp #$40
  bcc keydone
  
  lda doublejump
  cmp #$01
  beq keydone
  
  lda playerY
  cmp maxY
  beq j

  lda #$01
  sta doublejump

  
  j:
  lda playerY
  sec
  sbc #$40
  sta playerY
 
  lda #$00   ;reset key
  sta $ff
  

  rts  
  
drawscore:
  lda time
  clc
  adc #$01
  sta time
  
  ldx time
  lda #$06
  sta $0200,x
  rts 

end:
jsr update_platform
jsr draw_platform
jsr draw_player
jsr update_player

; jsr drawscore



jsr delay
jsr readkey

jmp end

gameover:
  lda #$00
  sta $0488
  lda #$04
  sta $04a8
  jsr delay
  lda #$00
  sta $04a8
  lda #$04
  sta $0508
  
  
