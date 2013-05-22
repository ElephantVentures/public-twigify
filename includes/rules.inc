<?php

$selfContainedRules = array(
  // sigh; some themes have empty blocks (cough, garland, cough)
  'empty' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '',
  ),
  'empty;' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '',
  ),
  // comments
  'T_COMMENT' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_COMMENT   => '',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{#%2$s%1$s#}',
  ),
  'T_DOC_COMMENT' => array(
    'pattern' => array(
      T_OPEN_TAG    => '<?php',
      T_DOC_COMMENT => '',
      T_CLOSE_TAG   => '?>',
    ),
    'format' => '{#%2$s%1$s#}',
  ),
  // look for T_OPEN_TAG (T_PRINT|T_ECHO) [(] (T_VARIABLE|T_CONSTANT_ENCAPSED_STRING) [)] [;] T_CLOSE_TAG
  // ignore echo w/multiple arguments for now
  'print $var' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_VARIABLE  => '',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print $var;' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_VARIABLE  => '',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print($var)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print($var);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo $var' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_VARIABLE  => '',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo $var;' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_VARIABLE  => '',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo($var)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo($var);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  // print constant string and variants
  'print const_string' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print const_string;' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print(const_string)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print(const_string);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo const_string' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo const_string;' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo(const_string)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo(const_string);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  // print t(constant string) and variants; ignore substitution by t for now
  'print t(const_string)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_STRING    => 't',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s|t }}',
  ),
  'print t(const_string);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_STRING    => 't',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s|t }}',
  ),
  'print(t(const_string))' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_OPEN_PARENTHESIS => '(',
      T_STRING    => 't',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s|t }}',
  ),
  'print(t(const_string));' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_OPEN_PARENTHESIS => '(',
      T_STRING    => 't',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s|t }}',
  ),
  'echo t(const_string)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_STRING    => 't',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s|t }}',
  ),
  'echo t(const_string);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_STRING    => 't',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s|t }}',
  ),
  'echo(t(const_string))' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_OPEN_PARENTHESIS => '(',
      T_STRING    => 't',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s|t }}',
  ),
  'echo(t(const_string));' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_OPEN_PARENTHESIS => '(',
      T_STRING    => 't',
      T_OPEN_PARENTHESIS => '(',
      T_CONSTANT_ENCAPSED_STRING => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s|t }}',
  ),
  // print render($var)[;]
  'print render($var)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_STRING    => 'render',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print render($var);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_STRING    => 'render',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print(render($var))' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_OPEN_PARENTHESIS => '(',
      T_STRING    => 'render',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'print(render($var));' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_PRINT     => 'print',
      T_OPEN_PARENTHESIS => '(',
      T_STRING    => 'render',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo render($var)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_STRING    => 'render',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo render($var);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_STRING    => 'render',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo(render($var))' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_OPEN_PARENTHESIS => '(',
      T_STRING    => 'render',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  'echo(render($var));' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ECHO      => 'echo',
      T_OPEN_PARENTHESIS => '(',
      T_STRING    => 'render',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{{ %s }}',
  ),
  // show/hide/unset a variable
  'show($var)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_STRING    => 'show',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% show(%s) %%}',
  ),
  'show($var);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_STRING    => 'show',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% show(%s) %%}',
  ),
  'hide($var)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_STRING    => 'hide',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% hide(%s) %%}',
  ),
  'hide($var);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_STRING    => 'hide',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% hide(%s) %%}',
  ),
  'unset($var)' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_STRING    => 'unset',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% unset(%s) %%}',
  ),
  'unset($var);' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_STRING    => 'unset',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% unset(%s) %%}',
  ),
  // if (): elseif ():, else if ():, else:, and endif[;]
  'if (boolean_expression):' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_IF        => 'if',
      T_BOOLEAN_EXPR => '',
      T_COLON     => ':',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% if %s %%}',
  ),
  'elseif (boolean_expression):' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ELSEIF    => 'elseif',
      T_BOOLEAN_EXPR => '',
      T_COLON     => ':',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% elseif %s %%}',
  ),
  'else if (boolean_expression):' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ELSE      => 'else',
      T_IF        => 'if',
      T_BOOLEAN_EXPR => '',
      T_COLON     => ':',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% elseif %s %%}',
  ),
  'else:' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ELSE      => 'else',
      T_COLON     => ':',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% else %%}',
  ),
  'endif' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ENDIF     => 'endif',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% endif %%}',
  ),
  'endif;' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ENDIF     => 'endif',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% endif %%}',
  ),
  // foreach (): and endfor[;]
  'foreach ($var as $val):' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_FOREACH   => 'foreach',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_AS        => 'as',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_COLON     => ':',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% for %2$s in %1$s %%}',
  ),
  'foreach ($var as $key => $val):' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_FOREACH   => 'foreach',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE  => '',
      T_AS        => 'as',
      T_VARIABLE  => '',
      T_DOUBLE_ARROW => '=>',
      T_VARIABLE  => '',
      T_CLOSE_PARENTHESIS => ')',
      T_COLON     => ':',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% for %2$s, %3$s in %1$s %%}',
  ),
  'endforeach' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ENDFOREACH => 'endforeach',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% endfor %%}',
  ),
  'endforeach;' => array(
    'pattern' => array(
      T_OPEN_TAG  => '<?php',
      T_ENDFOREACH => 'endforeach',
      T_SEMICOLON => ';',
      T_CLOSE_TAG => '?>',
    ),
    'format' => '{%% endfor %%}',
  ),
);

$openBraceRules = array(
  'if (boolean_expression) {' => array(
    'pattern' => array(
      T_OPEN_TAG   => '<?php',
      T_IF         => 'if',
      T_BOOLEAN_EXPR => '',
      T_OPEN_BRACE => '{',
      T_CLOSE_TAG  => '?>',
    ),
    'format' => '{%% if %s %%}',
  ),
  '} elseif (boolean_expression) {' => array(
    'pattern' => array(
      T_OPEN_TAG   => '<?php',
      T_CLOSE_BRACE => '}',
      T_ELSEIF     => 'elseif',
      T_BOOLEAN_EXPR => '',
      T_OPEN_BRACE => '{',
      T_CLOSE_TAG  => '?>',
    ),
    'format' => '{%% elseif %s %%}',
  ),
  '} else if (boolean_expression) {' => array(
    'pattern' => array(
      T_OPEN_TAG   => '<?php',
      T_CLOSE_BRACE => '}',
      T_ELSE       => 'else',
      T_IF         => 'if',
      T_BOOLEAN_EXPR => '',
      T_OPEN_BRACE => '{',
      T_CLOSE_TAG  => '?>',
    ),
    'format' => '{%% elseif %s %%}',
  ),
  '} else {' => array(
    'pattern' => array(
      T_OPEN_TAG   => '<?php',
      T_CLOSE_BRACE => '}',
      T_ELSE       => 'else',
      T_OPEN_BRACE => '{',
      T_CLOSE_TAG  => '?>',
    ),
    'format' => '{%% else %%}',
  ),
  'foreach ($var as $val) {' => array(
    'pattern' => array(
      T_OPEN_TAG   => '<?php',
      T_FOREACH    => 'foreach',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE   => '',
      T_AS         => 'as',
      T_VARIABLE   => '',
      T_CLOSE_PARENTHESIS => ')',
      T_OPEN_BRACE => '{',
      T_CLOSE_TAG  => '?>',
    ),
    'format' => '{%% for %2$s in %1$s %%}',
  ),
  'foreach ($var as $key => $val) {' => array(
    'pattern' => array(
      T_OPEN_TAG   => '<?php',
      T_FOREACH    => 'foreach',
      T_OPEN_PARENTHESIS => '(',
      T_VARIABLE   => '',
      T_AS         => 'as',
      T_VARIABLE   => '',
      T_DOUBLE_ARROW => '=>',
      T_VARIABLE   => '',
      T_CLOSE_PARENTHESIS => ')',
      T_OPEN_BRACE => '{',
      T_CLOSE_TAG  => '?>',
    ),
    'format' => '{%% for %2$s, %3$s in %1$s %%}',
  ),
);

$closeBraceRules = array(
  '}' => array(
    'pattern' => array(
      T_OPEN_TAG    => '<?php',
      T_CLOSE_BRACE => '}',
      T_CLOSE_TAG   => '?>',
    ),
    'format' => '{%% end %%}', // will tr end to endif or endfor in apply_rules
  ),
  '};' => array(
    'pattern' => array(
      T_OPEN_TAG    => '<?php',
      T_CLOSE_BRACE => '}',
      T_SEMICOLON   => ';',
      T_CLOSE_TAG   => '?>',
    ),
    'format' => '{%% end %%}', // will tr end to endif or endfor in apply_rules
  ),
);
