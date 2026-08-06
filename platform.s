define plat0 $00
define plat1 $01
define plat2 $02
define plat3 $03

define temp0 $04
define temp1 $05
define temp2 $06
define temp3 $07

define seed $08   ; currently just 1 byte

define playerY $0a

; init plat 
lda #$aa
sta plat0
lda #$ea
sta plat1
sta plat2
lda #$fe
sta plat3

; init playerY
lda #$80   
sta playerY


; init seed
lda #%01100101
sta seed

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
  lda plat2 
  sta temp2
  lda plat3 
  sta temp3
  rts

draw_platform:
  jsr load_temp
  ldx #$00
  
  platLoop:
      lsr temp3    ; outputs into carry
      ror temp2    ; input = carry
      ror temp1    ; input = carry
      ror temp0    ; input = carry
  
      bcc hole
    
  plat:
    lda #$05         ;green
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
  nextbit:
    inx
    cpx #$20
    bne platLoop
    rts
  
update_platform:
  jsr random_num
  ror plat3
  ror plat2
  ror plat1
  ror plat0
  rts


random_num:
  lda $fe
  cmp #$80
  rol
  rts
  


delay:
  ldx #$ff
  del1:
    lda #$03
    sta $0201
    dex
    bne del1
    rts

update_player:
  lda playerY
  cmp #$00
  beq update_player_done
  sec 
  sbc #$20
  sta playerY

  update_player_done:
  rts
  
draw_player:
  ldx #$00
  lda #$00
  clear_top:
    sta $03e0,x
    tay               ; x: num     y: a    a: a
    txa               ; x: num     y: a    a: x
    clc
    adc #$20
    tax
    tya
    cpx #$80
    bne clear_top
    
  
  lda #$04
  ldx playerY
  sta $03e8,x
  sta $0408,x
  rts

end:
jsr update_platform
jsr draw_platform
jsr update_player
jsr draw_player
jsr delay
jmp end
