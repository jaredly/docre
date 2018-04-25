
[@bs.val] [@bs.scope "ocaml"] external reason_compile : string => 'result = "reason_compile_super_errors";
[@bs.val] [@bs.scope "ocaml"] external ocaml_compile : string => 'result = "compile_super_errors";
[@bs.val] [@bs.scope "Colors"] external parseAnsi : string => {. "spans": array({. "text": string, "css": string})} = "parse";
[@bs.val] external eval: string => 'a = "";

[@bs.val] external bsRequirePaths: Js.Dict.t(string) = "";
[@bs.val] external packRequire: string => 'package = "";

[@bs.set "value"] external setInput: Dom.element => string => unit = "";
[@bs.send] external select: Dom.element => unit = "";
[@bs.scope "document"] [@bs.val] external execCommand: string => unit = "";

[@bs.val] [@bs.scope "location"] external origin: string = "";
[@bs.val] [@bs.scope "location"] external pathname: string = "";
[@bs.val] [@bs.scope "location"] external href: string = "";

[@bs.val] [@bs.module "lz-string"] external compress: string => string = "compressToEncodedURIComponent";
[@bs.val] [@bs.module "lz-string"] external decompress: string => string = "decompressFromEncodedURIComponent";
[@bs.val] [@bs.scope "history"] external replaceState: Js.Dict.t('a) => string => string => unit = "";

let replaceState = url => replaceState(Js.Dict.empty(), "", url);

let getInputValue = event => ReactDOMRe.domElementToObj(ReactEventRe.Form.target(event))##value;

type ast;
[@bs.val] external parseML: string => ast = "";
[@bs.val] external printML: ast => string = "";
[@bs.val] external parseRE: string => ast = "";
[@bs.val] external printRE: ast => string = "";

type codemirror;
[@bs.send] external setValue: (codemirror, string) => unit = "";
[@bs.send] external getValue: (codemirror) => string = "";
type textarea = Dom.element;

type jspos = {. "line": int, "ch": int};
[@bs.send] external markText: (codemirror, jspos, jspos, {. "className": string}) => unit = "";

let clearMarks: codemirror => unit = [%bs.raw {|
  (function(cm) {
    cm.getAllMarks().forEach(mark => {
      cm.removeLineWidget(mark)
    })
  })
|}];

let fromTextArea: (. textarea, string => unit) => codemirror = [%bs.raw {|
  function(textarea, onRun) {
    var betterShiftTab = /*onInfo => */cm => {
      var cursor = cm.getCursor()
        , line = cm.getLine(cursor.line)
        , pos = {line: cursor.line, ch: cursor.ch}
      cm.execCommand('indentLess')
    }

    var betterTab = /*onComplete => */cm => {
      if (cm.somethingSelected()) {
        return cm.indentSelection("add");
      }
      const cursor = cm.getCursor()
      const line = cm.getLine(cursor.line)
      const pos = {line: cursor.line, ch: cursor.ch}
      cm.replaceSelection(Array(cm.getOption("indentUnit") + 1).join(" "), "end", "+input");
    }

    var run = function(cm) {
      onRun(cm.getValue())
    }

    var toggleComment = cm => {
      var options = {indent: true}
      var minLine = Infinity, ranges = cm.listSelections(), mode = null;
      for (var i = ranges.length - 1; i >= 0; i--) {
        var from = ranges[i].from(), to = ranges[i].to();
        if (from.line >= minLine) continue;
        if (to.line >= minLine) to = Pos(minLine, 0);
        minLine = from.line;
        if (mode == null) {
          if (cm.uncomment(from, to, options)) mode = "un";
          else { cm.blockComment(from, to, options); mode = "line"; }
        } else if (mode == "un") {
          cm.uncomment(from, to, options);
        } else {
          cm.blockComment(from, to, options);
        }
      }

    var cm = CodeMirror.fromTextArea(textarea, {
      lineNumbers: true,
      lineWrapping: true,
      viewportMargin: Infinity,
      extraKeys: {
        'Cmd-Enter': (cm) => onRun(cm.getValue()),
        'Ctrl-Enter': (cm) => onRun(cm.getValue()),
        "Cmd-/": toggleComment
        },
        Tab: betterTab,
        'Shift-Tab': betterShiftTab,
      },
      mode: 'rust',
    })
    return cm
  }
|}];


let htmlEscape = text => text
|> Js.String.replace("&", "&amp;")
|> Js.String.replace("<", "&lt;")
|> Js.String.replace(">", "&gt;")
;

let fixEscapes = message => message
|> Js.String.replaceByRe(Js.Re.fromStringWithFlags({js|\u001b\\[1;|js}, ~flags="g"), {js|\u001b[|js})
|> Js.String.replaceByRe(Js.Re.fromStringWithFlags({js|\u001b\\[0m|js}, ~flags="g"), {js|\u001b[39m|js})
|> parseAnsi
|> result => result##spans
|> Array.map(span => {
  let css = span##css;
  let text = htmlEscape(span##text);
  Js.log2("span", span);
  {j|<span style="$css">$text</span>|j}
}) |> Js.Array.joinWith("");

type pos = {
  row: int,
  column: int
};
let jsPos = ({row, column}) => {"line": row, "ch": column};


type result('a, 'b) =
  | Ok('a)
  | Error('b);

let reasonCompile = code => {
  let result = reason_compile(code);
  switch (Js.Nullable.toOption(result##js_code)) {
  | Some(js) => Ok(js)
  | None =>
    Error((
      result##js_error_msg,
      {row: result##row, column: result##column},
      {row: result##endRow, column: result##endColumn}
    ))
  };
};

let ocamlCompile = code => {
  let result = ocaml_compile(code);
  switch (Js.Nullable.toOption(result##js_code)) {
  | Some(js) => Ok(js)
  | None =>
    Error((
      result##js_error_msg,
      {row: result##row, column: result##column},
      {row: result##endRow, column: result##endColumn}
    ))
  };
};