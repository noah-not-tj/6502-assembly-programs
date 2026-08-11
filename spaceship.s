; space craft thing


define enemy1_x  $00
define enemy1_y  $01
define enemy2_x  $02
define enemy2_y  $03
define enemy3_x  $04
define enemy3_y  $05

define player_x  $06

define temp1     $07 
define temp2     $08

define screenL $09
define screenH $0a

define enemy1_v $0b
define enemy2_v $0c
define enemy3_v $0d


; init
lda #$0a
sta enemy3_x
lda #$14
sta enemy2_x
lda #$0e
sta enemy1_x

lda #$00
sta enemy3_y
lda #$02
sta enemy2_y
lda #$01
sta enemy1_y

lda #$01
sta enemy3_v
lda #$02
sta enemy2_v
sta enemy1_v

lda #$10
sta player_x

lda #$00
sta screenL
lda #$02
sta screenH


jsr draw_bg
jmp main

draw_bg:
  ldx #$08
  stx screenL
  jsr draw_bgCol
  ldx #$18
  stx screenL
  jsr draw_bgCol
  jsr draw_bottom_bg
  rts

draw_bottom_bg:
  ldx #$05
  stx screenH
  ldx #$00  
  stx screenL
  
  ldy #$08
  lda #$0e
  barL:
    sta (screenL),y
    iny
    cpy #$19
    bne barL
rts

; draws col at screenL
draw_bgCol:
  ldx #$02
  stx screenH
  outbg:
    ldy #$00
    lda #$0e
      inbg:
        sta (screenL),y
        tax
	tya
        clc
        adc #$20
        tay
        txa
        cpy #$00
        bne inbg
    
      ldx screenH
      inx
      stx screenH
      cpx #$05
      bne outbg
      
      rts
  

update_enemies:
  lda enemy1_y
  clc
  adc enemy1_v 
  sta enemy1_y
  
  lda enemy2_y
  clc
  adc enemy2_v 
  sta enemy2_y
  
  lda enemy3_y
  clc
  adc enemy3_v 
  sta enemy3_y
  rts

;input is in a reg, output page is in temp1, frac is in a reg
yval_to_screenval:
  ldx #$02        ; accumulator
  
  inLoop:
    sec
    sbc #$08         ; subtract a column
    bcc doneloopy    ; if a goes negative we are done
    
    inx 
    cpx #$05         ; check if we are out of bounds
    beq toofary
    jmp inLoop
    
  doneloopy:
    adc #$08         ; restore a to positive 
    asl              ; shift over to multiply by 32
    asl
    asl
    asl
    asl 
    stx temp1
    rts
  
  toofary:
    lda #$00
    ldx #$05
    stx temp1
    rts

; input = color in the a reg, also screenvars and y = enemyx
draw_pixel:
  sta (screenL),y
  rts

draw_enemies:
  
  lda enemy3_y            
  jsr yval_to_screenval 
  sta screenL
  ldy enemy3_x
  ldx temp1
  stx screenH
  lda #$01
  jsr draw_pixel

  lda enemy3_y     
  sec
  sbc enemy3_v       
  jsr yval_to_screenval 
  sta screenL
  ldy enemy3_x
  ldx temp1
  stx screenH
  lda #$00
  jsr draw_pixel


  lda enemy2_y            
  jsr yval_to_screenval 
  sta screenL
  ldy enemy2_x
  ldx temp1
  stx screenH
  lda #$01
  jsr draw_pixel

  lda enemy2_y     
  sec
  sbc enemy2_v       
  jsr yval_to_screenval 
  sta screenL
  ldy enemy2_x
  ldx temp1
  stx screenH
  lda #$00
  jsr draw_pixel

  lda enemy1_y            
  jsr yval_to_screenval 
  sta screenL
  ldy enemy1_x
  ldx temp1
  stx screenH
  lda #$01
  jsr draw_pixel

  lda enemy1_y     
  sec
  sbc enemy1_v        
  jsr yval_to_screenval 
  sta screenL
  ldy enemy1_x
  ldx temp1
  stx screenH
  lda #$00
  jsr draw_pixel

  rts

draw_player:
  lda #$04
  ldx player_x
  sta $0480,x
  rts


delay:
  ldy #$01
  del2:
  ldx #$ff
  del1:
    lda #$00
    nop
    dex
    bne del1
  dey
  bne del2
  rts
  
  
main:
;  read_key
;  clear enemies
;  clear player
;  update_player
;  
  jsr update_enemies
  jsr draw_enemies
  jsr draw_player
  jsr delay
  jmp main
