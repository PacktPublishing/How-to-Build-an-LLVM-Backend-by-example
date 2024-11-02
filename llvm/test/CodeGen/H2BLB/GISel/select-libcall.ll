define void @scalarizeAndCallLib(ptr %src, ptr %dst) {
  %val = load <2 x float>, ptr %src
  %res = fadd <2 x float> %val, %val
  store <2 x float> %res, ptr %dst
  ret void
}
