define plat0 $00
define plat1 $01

define temp0 $04
define temp1 $05

define rand $fe

define playerY $0a
define maxY $0b

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

; init playerY
lda #$80
sta maxY
sta playerY

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
      lsr temp1    ; input = carry
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
  ldx #$ff
  del1:
    lda #$03
    sta $0201
    nop
    dex
    bne del1
    rts

update_player:
  lda playerY
  cmp maxY
  beq update_player_done
  clc
  adc #$20
  sta playerY

  update_player_done:
  rts
  
draw_player:

  ldx playerY
  cpx #$00          ; if already at top don't clear top
  beq draw_p
  
  ldx #$00     ; initial
  lda #$00     ; color
  clear_top:
    sta $03e8,x    ; clear pixel
    tay               ; x: num     y: a    a: a
    txa               ; x: num     y: a    a: x
    clc
    adc #$20       ; increment counter by 1 row (32)
    tax
    tya
    cpx playerY    ; check if it is at the height (draws height - 32)
    bne clear_top
    
  draw_p:  
    lda #$04
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
  sec
  sbc #$60
  sta playerY
 
  lda #$00   ;reset key
  sta $ff
  rts  
 

end:
jsr update_platform
jsr draw_platform
jsr update_player
jsr draw_player
jsr delay

jsr readkey

jmp end
