DR.dlopen('bottom_band') unless Object.const_defined?(:BottomBand)

def tick args
  if args.inputs.keyboard.key_down.r
    BottomBand.restore
  elsif !BottomBand.configure
    raise 'BottomBand: no usable macOS window or screen work area'
  end

  # This demo is opaque. The desktop is visible outside the physical window.
  args.outputs.background_color = [38, 65, 78]
end
