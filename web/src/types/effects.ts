import { EffectPreset } from './device'

export type EffectCategory = 'chase' | 'strobe' | 'random' | 'pattern' | 'fill' | 'wave'

export type AnimationPatternKey =
  | 'chase_left'
  | 'chase_right'
  | 'ping_pong'
  | 'center_out'
  | 'edges_in'
  | 'zip_unzip'
  | 'dual_snake'
  | 'binary_counter'
  | 'sparkle'
  | 'twinkle'
  | 'wave_fill'
  | 'heartbeat'
  | 'cylon'
  | 'alternating'
  | 'dominos'
  | 'marquee'
  | 'raindrop'
  | 'fireworks'
  | 'knight_rider'
  | 'meteor'
  | 'double_bounce'
  | 'cross_over'
  | 'strobe_burst'
  | 'stack_build'
  | 'reverse_stack'
  | 'quad_runner'
  | 'in_out_breathe'
  | 'segment_stride'
  | 'police_strobe'
  | 'curtain_reveal'

export interface EffectMeta {
  id: number
  name: string
  description: string
  category: EffectCategory
  defaultSpeed: number
  tags: string[]
  animationPattern: AnimationPatternKey
}

export type EffectLibrary = EffectPreset[]
