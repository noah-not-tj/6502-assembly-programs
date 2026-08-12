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

define screenL   $09
define screenH   $0a

define enemy1_v  $0b
define enemy2_v  $0c
define enemy3_v  $0d
 
define speed     $0e

define time      $0f ; two bytes
define time2     $10

define gamedone  $11


; init
lda #$0a
sta enemy3_x
lda #$14
sta enemy2_x
lda #$0e
sta enemy1_x

lda #$0d
sta enemy3_y
lda #$07
sta enemy2_y
lda #$00
sta enemy1_y

lda #$01
sta enemy3_v
sta enemy2_v
sta enemy1_v

lda #$10
sta player_x

lda #$00
sta screenL
lda #$02
sta screenH

lda #$02
sta speed

lda #$00
sta time
sta time2
sta gamedone


jsr draw_bg
jmp main

draw_bg:
  ldx #$07
  stx screenL
  jsr draw_bgCol
  ldx #$18
  stx screenL
  jsr draw_bgCol
  lda #$0e
  ldx #$00  
  stx screenL
  jsr draw_bottom_bg


  rts

draw_bottom_bg:
  ldx #$05
  stx screenH

  
  ldy #$07
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
  cmp #$19
  bcs reset1  
  sta enemy1_y
  jmp skipreset1

  reset1:
  lda #$01
  sta enemy1_v
  sta enemy1_y
  lda $fe
  and #$0f
  clc
  adc #$08 
  sta enemy1_x
  
  skipreset1:
  
  lda enemy2_y
  clc
  adc enemy2_v
  cmp #$19
  bcs reset2  
  sta enemy2_y
  jmp skipreset2

  reset2:
  lda #$01
  sta enemy2_v
  lda #$00
  sta enemy2_y
  lda $fe
  and #$0f
  clc
  adc #$08  
  sta enemy2_x
  
  skipreset2:
 
  lda enemy3_y
  clc
  adc enemy3_v
  cmp #$19
  bcs reset3  
  sta enemy3_y
  jmp skipreset3

  reset3:
  lda #$01
  sta enemy3_v
  lda #$00
  sta enemy3_y
  lda $fe
  and #$0f
  clc
  adc #$08 
  sta enemy3_x
  
  skipreset3:
  rts

;input is in a reg, output page is in temp1, frac is in a reg
yval_to_screenval:
  ldx #$02        ; accumulator
  
  inLoop:
    sec
    sbc #$08         ; subtract a column
    bcc doneloopy    ; if a goes negative we are done
    
    inx 
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
  ldx player_x

  pha
  lda $0460,x
  cmp #$01
  beq gover
  lda $047f,x
  cmp #$01
  beq gover
  lda $0480,x
  cmp #$01
  beq gover
  lda $0481,x
  cmp #$01
  beq gover

  pla

  sta $0460,x
  sta $047f,x
  sta $0480,x
  sta $0481,x
  rts

gover:
  pla
  ldx #$ff
  stx gamedone
  rts
  


delay:
  ldy #$01
  del2:
  ldx speed
  del1:
    lda #$00
    dex
    bne del1
  dey
  bne del2
  lda time
  clc
  adc #$01
  sta time
  bcc dd
  lda time2
  adc #$00
  sta time2
  dd:
  rts

draw_time:
  ldx time2
  lda time2
  sta $0540,x
  rts
  

read_key:
  ldx player_x
  cpx #$08
  bcc keydone
  cpx #$18
  bcs keydone
  lda $ff
  cmp #$61
  beq left
  cmp #$64
  beq right
keydone:
  ldx #$00
  stx $ff
  rts

  left: 
    lda #$00
    jsr draw_player
    ldx player_x
    dex
    stx player_x
    jmp dondye

  right: 
    lda #$00
    jsr draw_player
    ldx player_x
    inx 
    stx player_x
    jmp dondye

  dondye:
    ldx #$00  
    stx $ff
    rts
  
  
main:
  jsr read_key
  jsr update_enemies
  jsr draw_enemies
  lda #$04
  jsr draw_player

  lda gamedone
  cmp #$ff
  beq gameoverhaha

  lda #$0e
  ldx #$00  
  stx screenL
  jsr draw_bottom_bg

  jsr draw_time
  jsr delay

  jmp main

gameoverhaha:
  
