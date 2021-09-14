import oui, nimgl/glfw/native, winim, threadpool, osproc, strutils


proc map*(window: UiNode): UiNode =
  assert window.kind == UiWindow
  var p = startProcess("ouimap.exe", "", [$cast[int](getWin32Window(window.handle))], nil, {poUsePath, poStdErrToStdOut})
  for line in p.lines:
    let l = line.split(':')
    if l.len == 2:
      result.winid = parse_int(l[1])
  result = UiNode.init(UiEmbedded)
  p.close()


window:
  pressed:
    self.add map(self)
  w 500
  h 400
  color rgb(100, 0,  100)
  self.show()