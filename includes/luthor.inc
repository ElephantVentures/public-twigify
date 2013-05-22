<?php

/**
 * The goal is to translate from one language (PHP) to another (Twig).  Semantic analysis on
 * an abstract syntax tree (AST) would be easiest for such transformations, right?  However,
 * there are no well-developed tools for producing an AST from PHP in PHP.
 * 
 * The PHP internals use C.  Their "parser" also does a bit of semantic analysis and code
 * generation (see Parsekit).  It doesn't quite work for our purposes.
 * 
 * One of the main goals of Twig is to enable human-readable, easy-to-maintain templates.
 * Part and parcel of that goal is preserving whitespace and comments.  However, a traditional
 * compiler/translator will ditch whitespace and comments the first chance it gets; its goal
 * it to produce machine code (and not human-readable code).
 * 
 * If we want to preserve some of the layout (whitespace and comments) of the original PHP,
 * we need an AST from a parser that preserves (or is aware of) such things in the token
 * stream.  This requirement (along with bulding a PHP parser in PHP) means we have to grow
 * our tool ourselves.  NB: a concrete syntax tree would probably do just as well as an AST
 * here.
 * 
 * What follows is a crude lexer+.  It weakly parses some elements of PHP, and applies some
 * semantic understanding to transform a few expressions from PHP to Twig.  YMMV.
 */


/**** helper functions for processing PHP ****/

/**
 * Generate an error message in a Twig comment when we hit a token
 * we didn't expect to see during processing.
 */
function unexpected_token_message($token) {
  $tokenName    = token_name($token[0]);
  $tokenText    = strtr($token[1], array("\t" => '\t', "\n" => '\n'));
  $lineNumber   = $token[2];
  $errorMessage = "ERROR: Encountered unexpected token $tokenName on line $lineNumber: $tokenText";
  return "{# $errorMessage #}";
}


/**
 * token_get_all returns a list of tokens, but some tokens
 * that are single characters get returned raw (and not as
 * an array of name, token, lineNumber).  we do three things:
 * convert all singlets to arrays, ensure a closing tag if
 * we're in the middle of a PHP block at EOF (valid PHP
 * doesn't require one), and change the list to a stack.
 */
define("T_OPEN_PARENTHESIS",  4913);
define("T_CLOSE_PARENTHESIS", 4914);
define("T_COMMA",        4915);
define("T_SEMICOLON",    4916);
define("T_OPEN_BRACKET", 4917);
define("T_CLOSE_BRACKET", 4918);
define("T_QUESTION",     4919);
define("T_COLON",        4920);
define("T_BOOLEAN_NOT",  4921);
define("T_LESS_THAN",    4922);
define("T_GREATER_THAN", 4923);
define("T_ASSIGNMENT",   4924);
define("T_PLUS",         4925);
define("T_MINUS",        4926);
define("T_CONCAT",       4927);
define("T_MUL",          4928);
define("T_DIV",          4929);
define("T_MOD",          4930);
define("T_BIT_NOT",      4931);
define("T_BIT_AND",      4932);  define("T_REFERENCE", 4932);  define("T_AMPERSAND", 4932);
define("T_BIT_XOR",      4933);
define("T_BIT_OR",       4934);
define("T_IGNORE_ERROR", 4935);
define("T_OPEN_BRACE",   4936);
define("T_CLOSE_BRACE",  4937);
function preprocess_tokens($tokenList) {
  $tokenTypes = array(
    '(' => T_OPEN_PARENTHESIS,
    ')' => T_CLOSE_PARENTHESIS,
    ',' => T_COMMA,
    ';' => T_SEMICOLON,
    '[' => T_OPEN_BRACKET,
    ']' => T_CLOSE_BRACKET,
    '?' => T_QUESTION,
    ':' => T_COLON,
    '!' => T_BOOLEAN_NOT,
    '<' => T_LESS_THAN,
    '>' => T_GREATER_THAN,
    '=' => T_ASSIGNMENT,
    '+' => T_PLUS,
    '-' => T_MINUS,
    '.' => T_CONCAT,
    '*' => T_MUL,
    '/' => T_DIV,
    '%' => T_MOD,
    '~' => T_BIT_NOT,
    '&' => T_BIT_AND, // sigh, depends on context: binary "&" is T_BIT_AND where unary "&" is T_REFERENCE
    '^' => T_BIT_XOR,
    '|' => T_BIT_OR,
    '@' => T_IGNORE_ERROR,
    '{' => T_OPEN_BRACE,
    '}' => T_CLOSE_BRACE,
  );
  $result = array();
  $lineNumber = 1;
  foreach ($tokenList as $token) {
    if ( gettype($token) !== 'array' ) {
      $token = array(
        $tokenTypes[$token],
        $token,
        $lineNumber, // get from previous token (FIXME: could be off if previous token has trailing whitespace that contains line breaks)
      );
    } else {
      $lineNumber = $token[2];
    }
    array_push($result,$token);
  }
  if ($token[0] !== T_CLOSE_TAG && $token[0] !== T_INLINE_HTML) {
    array_push($result,array(
      T_CLOSE_TAG,
      '?>',
      $lineNumber, // get from previous token (which is guaranteed to exist if valid PHP)
    ));
  }
  $result = array_reverse($result);
  return $result;
}

/**
 * Filters whitespace from a list of tokens.  NB: this is done
 * in place (destructively); note the pass by reference.
 */
function filter_whitespace(&$tokenList) {
  $tokenList = array_filter(
    $tokenList,
    function ($item) {
      // kill whitespace but keep all others
      return ($item[0] === T_WHITESPACE) ? FALSE : TRUE;
    }
  );
}

/**
 * Grabs a block of PHP off the top of the stack, and returns it.  NB: note the
 * pass by reference; the incoming stack is modified as the PHP block is popped.
 */
function pop_php(&$tokenStack) {
  $result = array();
  // first token is always T_OPEN_TAG so pop right away
  do {
    $token = array_pop($tokenStack);
    array_push($result,$token);
  } while ($token[0] !== T_CLOSE_TAG);
  return array_reverse($result);
}

/**
 * Mashes together a token stack into the original string.
 */
function print_php($tokenStackToPrint) {
  $result = '';
  while ($token = array_pop($tokenStackToPrint)) {
    $type = $token[0];
    $name = $token[1];
    switch($type) {
      case T_OPEN_TAG:
        // PHP open tag to Twig open comment (and preserve whitespace)
        $name = strtr($name,array('<?php'=>'{# PHP:'));
        break;
      case T_CLOSE_TAG:
        // PHP close tag to Twig close comment (and preserve whitespace)
        $name = strtr($name,array('?>'=>'#}'));
        break;
      default:
        // pass through, but make sure not to close the Twig comment by mistake
        $name = strtr($name,array('#}'=>'#hash}brace'));
        break;
    }
    $result .= $name;
  }
  return $result;
}

/**
 * Breaks a block of PHP up into pieces.  The pieces aren't always statements.
 * Opening if/elseif/else, foreach are peeled off as are comments.  We return
 * an array of token stacks, one entry per subblock.
 */
function php_subblocks($tokenStack) {

  // a place to store the returned subblocks
  $subblocks = array();

  // the first token is always T_OPEN_TAG
  $openToken = array_pop($tokenStack);
  // preserve any trailing whitespace for the first subblock
  $whitespaces = explode('<?php',$openToken[1]);
  $firstWS = $whitespaces[1];
  $isFirst = TRUE;
  $lineNumber = $openToken[2];

  do {

    // the first token is always T_OPEN_TAG
    $subblock = array( array(
      T_OPEN_TAG,
      '<?php' . ($isFirst ? $firstWS : ''),
      $lineNumber,
    ));
    $isFirst = FALSE;

    // find the first non-whitespace token
    do {
      $token = array_pop($tokenStack);
      $type = $token[0];
      array_push($subblock,$token);
    } while ($type === T_WHITESPACE);

    // comments, close braces, and endif/endforeach go in their own subblock
    // as does the opening for if/elseif/else and foreach
    // LIE: everybody else is delimited by a semicolon or a T_CLOSE_TAG
    // FIXME: the above is wishful thinking; there are a gajillion cases to handle here,
    //        and really what we need is the parse tree (which'd identify blocks and
    //        statements); heck, even "for" and "switch" statements break this, as do
    //        closures -- all of which aren't unexpected, even in templates.
    switch ($type) {
      case T_COMMENT:
      case T_DOC_COMMENT:
      // case T_ML_COMMENT: is PHP4 only
        // do nothing; the comment has already been pushed, and whitespace is picked up below
        break;
      case T_ENDIF:
      case T_ENDFOREACH:
        // look ahead for the optional semicolon
        // if present, gobble it; if not, put back the look-ahead token
        $lookAheadToken = array_pop($tokenStack);
        $lookAheadType = $lookAheadToken[0];
        if ($lookAheadType === T_SEMICOLON) {
          array_push($subblock,$lookAheadToken);
        } else {
          array_push($tokenStack,$lookAheadToken);
        }
        break;
      case T_CLOSE_BRACE:
        // look ahead
        // if we see the optional semicolon, that's the end of the block: stop.
        // if we see anything but an else/elseif, we just finished an if or foreach: stop and put back lookahead
        // if we see an elseif, fall through to below to find the conditional
        // if we see an else, we'd like to jump to the T_ELSE case below (but goto doesn't allow that so we repeat code instead)
// FIXME: ugly but necessary; is there a cleaner way to do this?
$whitespaceStack = array();
do {
  $token = array_pop($tokenStack);
  $type = $token[0];
  array_push($whitespaceStack,$token);
} while ($type === T_WHITESPACE);
        $lookAheadToken = array_pop($whitespaceStack);
        $lookAheadType = $lookAheadToken[0];
        if ($lookAheadType === T_SEMICOLON) {
foreach ($whitespaceStack as $token) { array_push($tokenStack,$token); }
          array_push($subblock,$lookAheadToken);
          break;
        } elseif ($lookAheadType === T_ELSE) {
// whitespace gets ignored in an else subblock
          array_push($subblock,$lookAheadToken);
          // look for the opening brace or colon
          do {
            $token = array_pop($tokenStack);
            $type = $token[0];
            array_push($subblock,$token);
          } while ($type !== T_OPEN_BRACE);
          break;
        } elseif ($lookAheadType === T_ELSEIF) {  // FIXME??? does "else if" get converted to an T_ELSEIF token?
// whitespace gets ignored in an elseif subblock
          array_push($subblock,$lookAheadToken);
          // no break
        } else {
          array_push($tokenStack,$lookAheadToken);
while ($whitespaceStack) { array_push($tokenStack, array_pop($whitespaceStack)); }
          break;
        }
      case T_IF:
      case T_ELSEIF:
      case T_FOREACH:
        // look for the opening parenthesis on the condition / loop array
        do {
          $token = array_pop($tokenStack);
          $type = $token[0];
          array_push($subblock,$token);
        } while ($type !== T_OPEN_PARENTHESIS);
        // look for the matching close parenthesis
        $openParenCount = 1;
        do {
          $token = array_pop($tokenStack);
          array_push($subblock,$token);
          if ($token[0] === T_OPEN_PARENTHESIS) { $openParenCount++; }
          elseif ($token[0] === T_CLOSE_PARENTHESIS) { $openParenCount--; }
        } while ($openParenCount > 0);
        // no break; fall through to T_ELSE to get the brace or colon
      case T_ELSE:
        // look for the opening brace or colon
        do {
          $token = array_pop($tokenStack);
          $type = $token[0];
          array_push($subblock,$token);
        } while ($type !== T_OPEN_BRACE && $type !== T_COLON);
        break;
      default:
        // look for a semicolon or the close tag
        // NB: $type is from above; we check first because we may have hit an empty block
        while ($type !== T_SEMICOLON && $type !== T_CLOSE_TAG) {
          $token = array_pop($tokenStack);
          $type = $token[0];
          array_push($subblock,$token);
        }
        // if we stopped on the close tag, put it and any preceding whitespace back on
        if ($type === T_CLOSE_TAG) {
          $closeToken = array_pop($subblock);
          array_push($tokenStack,$closeToken);
          $lookBackToken = array_pop($subblock);
          if ($lookBackToken[0] === T_WHITESPACE) {
            array_push($tokenStack,$lookBackToken);
          } else {
            array_push($subblock,$lookBackToken);
          }
        }
        break;
    }

    // soak up whitespace to postpend to the T_CLOSE_TAG
    $whitespaceStack = array();
    do {
      $token = array_pop($tokenStack);
      $type = $token[0];
      array_push($whitespaceStack,$token);
    } while ($type === T_WHITESPACE);

    // we went one token too far; push it back and add T_CLOSE_TAG
    // (unless it was a T_CLOSE_TAG; then we just stop)
    if ($type !== T_CLOSE_TAG) {
      // push back the last token; it's the start of the next block
      $nextBlockToken = array_pop($whitespaceStack);
      array_push($tokenStack,$nextBlockToken);
      // add a T_CLOSE_TAG to the subblock, and glue the whitespace onto it
      $whitespace = '';
      foreach ($whitespaceStack as $token) {
        $whitespace .= $token[1];
      }
      $lineNumber = $token[2]; // update for T_OPEN_TAG at top of loop
      array_push($subblock,array(
        T_CLOSE_TAG,
        '?>' . $whitespace,
        $lineNumber,
      ));
    } else {
      // push whitespaceStack onto the subblock
      foreach ($whitespaceStack as $token) {
        array_push($subblock,$token);
      }
      // NB: tokenStack is exhausted; we don't need to update lineNumber
    }

    // add the subblock to the return
    $subblocks[] = array_reverse($subblock);

  } while ($tokenStack);

  // return what we found
  return $subblocks;
}


/**** functions to parse and transform variable globs ****/

/**
 * Grabs a variable glob off the top of the stack, and returns a Twigified version.
 * NB: note the pass by reference; the incoming stack is modified as the variable
 * glob is popped.
 */
function gobble_variable(&$tokenStack) {
  // tokens that we might see in a variable glob
  // NB: basically we stop on T_COMMA, an unbalanced T_CLOSE_PARENTHESIS or T_CLOSE_BRACKET, T_SEMICOLON, or T_CLOSE_TAG
  //     there are other possibilities, but those are the ones we understand for now
  $acceptTokenTypes =  array(
    T_VARIABLE,  // variable names
    T_OBJECT_OPERATOR,  // object access
    T_OPEN_PARENTHESIS, T_CLOSE_PARENTHESIS,  // object methods
    T_STRING,  // object methods and members
    T_OPEN_BRACKET, T_CLOSE_BRACKET,  // array access
    T_CONSTANT_ENCAPSED_STRING, T_LNUMBER,  // array keys
    // TODO: T_ENCAPSED_AND_WHITESPACE, T_NUM_STRING, T_STRING_VARNAME,  // array keys
  );
  // a place to hold the tokens we're gobbling
  $varTokens = array();

  // gobble, gobble; NB: first token is expected to be T_VARIABLE
  $bracketCount = 0; $parenthesisCount = 0;
  do {
    $token = array_pop($tokenStack);
    array_push($varTokens,$token);
    // make sure not to leave the tree node we started from
    if ( $token[0] === T_OPEN_PARENTHESIS ) { $parenthesisCount++; }
    if ( $token[0] === T_CLOSE_PARENTHESIS ) { $parenthesisCount--; }
    if ( $token[0] === T_OPEN_BRACKET ) { $bracketCount++; }
    if ( $token[0] === T_CLOSE_BRACKET ) { $bracketCount--; }
  } while ( in_array($token[0], $acceptTokenTypes, TRUE) &&
            $bracketCount >= 0 &&
            $parenthesisCount >= 0 );
  // we went one token too far; put it back
  // (which is OK because there's always T_CLOSE_TAG, T_CLOSE_PARENTHESIS or T_CLOSE_BRACKET, or whatever other non-accept token we hit)
  array_push( $tokenStack, array_pop($varTokens) );
// FIXME: unnecessary if we assume valid PHP, but we could check that the counts are both 0, or one 0 and one -1 (but no other state)
  // the T_VARIABLE that started is at the bottom of the stack; reverse the order
  $varTokens = array_reverse($varTokens);

  // start a recursive parse
  return _gobble_variable_recurse('',$varTokens);
}

/**
 * Recursively walk the token stack, processing PHP variables, object member and method
 * accesses, and array accesses as we go.
 *
 * object access:
 *   with variable as member/method name needs attribute(object,method/member)
 *   a method with arguments requires attribute(object,method,arguments)
 *   but methods with no arguments and T_STRING-identified members are easy
 * array access:
 *   array accesses with constant strings are easy
 *   variable array access needs attribute(array,item)
 * FIXME(?): we're gonna ignore variables that store functions for now; we will, however, correctly catch variables that store object methods
 */
function _gobble_variable_recurse($parentName,$varTokens) {
  $nextToken = array_pop($varTokens);
  if (!$nextToken) { return $parentName; }
  $type = $nextToken[0];
  $name = $nextToken[1];
  switch($type) {
    case T_VARIABLE:
      // we must be at the start; ignore the parent
      return _gobble_variable_recurse(substr($name,1),$varTokens);
      break;
    case T_OBJECT_OPERATOR:
      // object access
      $momToken = array_pop($varTokens); // member or method of object
      $momType = $momToken[0]; // T_VARIABLE or T_STRING (either of which can be a member or method name)
      $momName = $momToken[1];
      if ($momType === T_VARIABLE) { $momName = substr($momName,1); }
      $lookAheadToken = array_pop($varTokens);
      if (!$lookAheadToken) {
        // we hit the end; return the appropriate accessor
        switch ($momType) {
          case T_STRING:
            return "$parentName.$momName";
            break;
          case T_VARIABLE:
            return "attribute($parentName,$momName)";
            break;
          default:
            return unexpected_token_message($momToken);
            break;
        }
      }
      // keep going an analyze lookAhead
      $lookAheadType = $lookAheadToken[0];
      $lookAheadName = $lookAheadToken[1];
      switch ($lookAheadType) {
        case T_OPEN_PARENTHESIS:
          $lookAheadTwoToken = array_pop($varTokens);
          if ($lookAheadTwoToken[0] === T_CLOSE_PARENTHESIS) {
            // we hit an argumentless method
            switch ($momType) {
              case T_STRING:
                $varName = "$parentName.$momName";
                break;
              case T_VARIABLE:
                $varName = "attribute($parentName,$momName)";
                break;
              default:
                return unexpected_token_message($momToken);
                break;
            }
            return _gobble_variable_recurse($varName,$varTokens);
          } else {
            // we hit a method with arguments; back up and recurse
            array_push($lookAheadTwoToken);
            array_push($lookAheadToken);
            return _gobble_variable_recurse("$parentName,$momName",$varTokens);
          }
          break;
        case T_OPEN_BRACKET:
          // we hit an array access; back up and recurse
          array_push($varTokens,$lookAheadToken);
          switch ($momType) {
            case T_STRING:
              $varName = "$parentName.$momName";
              break;
            case T_VARIABLE:
              $varName = "attribute($parentName,$momName)";
              break;
            default:
              return unexpected_token_message($momToken);
              break;
          }
          return _gobble_variable_recurse($varName,$varTokens);
          break;
        default:
          return unexpected_token_message($lookAheadToken);
          break;
      }
      break;
    case T_OPEN_BRACKET:
      // array access
      $lookAheadToken = array_pop($varTokens);
      $lookAheadType = $lookAheadToken[0];
      $lookAheadName = $lookAheadToken[1];
      switch ($lookAheadType) {
        case T_LNUMBER:
          // pop the next token and check it's a closing bracket
          $lookAheadTwoToken = array_pop($varTokens);
          if ($lookAheadTwoToken[0] !== T_CLOSE_BRACKET) { return unexpected_token_message($lookAheadTwoToken); }
          // process the lookAhead token and recurse on the rest
          return _gobble_variable_recurse($parentName . "[$lookAheadName]", $varTokens);
          break;
        case T_CONSTANT_ENCAPSED_STRING:
          // pop the next token and check it's a closing bracket
          $lookAheadTwoToken = array_pop($varTokens);
          if ($lookAheadTwoToken[0] !== T_CLOSE_BRACKET) { return unexpected_token_message($lookAheadTwoToken); }
          // process the lookAhead token and recurse on the rest
          $lookAheadName = trim($lookAheadName,'\'"');
          return _gobble_variable_recurse("$parentName.$lookAheadName", $varTokens);
          break;
        case T_VARIABLE:
          // grab the variable glob
          array_push($varTokens,$lookAheadToken);
          $varName = gobble_variable($varTokens);
          // pop the next token and check it's a closing bracket
          $lookAheadTwoToken = array_pop($varTokens);
          if ($lookAheadTwoToken[0] !== T_CLOSE_BRACKET) { return unexpected_token_message($lookAheadTwoToken); }
          // process the lookAhead token and recurse on the rest
          return _gobble_variable_recurse("attribute($parentName,$varName)",$varTokens);
          break;
        default:
          return unexpected_token_message($lookAheadToken);
          break;
      }
      break;
    case T_OPEN_PARENTHESIS:
      // we hit a method with arguments; gobble up the arguments
      $args = array();
      $stop = FALSE;
      do {
        $args[] = gobble_variable($varTokens);
        $nextToken = array_pop($varTokens);
        // check: arguments should be separated by commas and delimited by parentheses
        if ($nextToken[0] !== T_COMMA && $nextToken[0] !== T_CLOSE_PARENTHESIS) { return unexpected_token_message($nextToken); }
        $stop = ($nextToken[0] === T_CLOSE_PARENTHESIS); // either comma or close parenthesis
      } while (!$stop);
      return _gobble_variable_recurse("attribute($parentName,$varName," . implode(',',$args) . ')', $varTokens);
      break;
    default:
      return unexpected_token_message($nextToken);
      break;
  }
}


/**** processing expressions (???) ****/

/**
 * Process a boolean expression -- variables and logical operators -- into
 * an equivalent Twig expression.  The function expects the expression to
 * be wrapped in parentheses (as for "if" statements).
 */
function gobble_boolean_expression(&$tokenStack) {

  // a place to hold the translation of the tokens we're gobbling
  $result = '';

  // gobble, gobble; NB: first token is expected to be T_OPEN_PARENTHESIS
  $parenthesisCount = 0;
  do {
    $token = array_pop($tokenStack);
    $type = $token[0];
    switch ($type) {
      case T_OPEN_PARENTHESIS:
        $parenthesisCount++;
        // if we're in the middle of the expression, add another parenthesis
        if ($parenthesisCount > 1) { $result .= '('; }
        break;
      case T_CLOSE_PARENTHESIS:
        $parenthesisCount--;
        // remove any trailing whitespace (may or may not be present)
        $result = trim($result);
        // if we're in the middle of the expression, add another parenthesis
        if ($parenthesisCount > 0) { $result .= ')'; }
        break;
      case T_VARIABLE:
        array_push($tokenStack,$token);
        $result .= gobble_variable($tokenStack) . ' ';
        break;
      case T_LESS_THAN:
        $result .= '< ';
        break;
      case T_GREATER_THAN:
        $result .= '> ';
        break;
      case T_IS_SMALLER_OR_EQUAL:
        $result .= '<= ';
        break;
      case T_IS_GREATER_OR_EQUAL:
        $result .= '>= ';
        break;
      case T_IS_EQUAL:
        $result .= '== ';
        break;
      case T_IS_NOT_EQUAL:
        $result .= '!= ';
        break;
      // case T_IS_IDENTICAL: case T_IS_NOT_IDENTICAL: FIXME: unsupported in Twig?
      case T_BOOLEAN_AND:
      case T_LOGICAL_AND:
        $result .= 'and ';
        break;
      case T_BOOLEAN_NOT:
        $result .= 'not ';
        break;
      case T_BOOLEAN_OR:
      case T_LOGICAL_OR:
        $result .= 'or ';
        break;
      // case T_LOGICAL_XOR: FIXME: Twig doesn't have this (only b-xor); implement as (x or y) and not (x and y)?
      case T_DNUMBER:
        $result .= $token[1] . ' ';
        break;
      case T_LNUMBER:
        $result .= $token[1] . ' ';
        break;
      case T_CONSTANT_ENCAPSED_STRING:
        $result .= $token[1] . ' ';
        break;
      // function and function-like calls
      case T_STRING:
      case T_EMPTY:
      case T_ISSET:
        $result .= $token[1];
        break;
      default:
        $result .= unexpected_token_message($token);
        break;
      // FIXME: should we accept arithmetic (+ - * / % ~ & ^ | << >>) and casts?  assignments?  -- and ++?  what else?
    }
  } while ( $tokenStack && $parenthesisCount > 0 );

  // there should always be at least a T_CLOSE_TAG left
  // ALT: if tokenStack is empty *and* we didn't end on a close paren, then throw an error
  if (count($tokenStack) == 0) { return "{# ERROR: Unexpected end of token stream at line {$token[2]} #}"; }

  return $result;

}


/**
 * Returns the parameters from a call to theme().
 *
 * We assume that a T_STRING has already been popped off the stack with text "theme".
 *
 * @param array $tokenStack
 *   the token stack after T_STRING has been popped off; passed by value
 *
 * @return array
 *   hook: the first argument to the theme() call, the hook name
 *   variables: (optional) an array of variables passed to theme()
 */
function twig_theme_call_parameters($tokenStack) {
  // first token should be an open parenthesis for the function call
  $parenToken = array_pop($tokenStack);
  if ($parenToken[0] !== T_OPEN_PARENTHESIS) { return array('hook'=>'ERROR: missing open parenthesis to theme() call'); }

  // look for the first argument, the hook name
  // though this could be any expression, for simplicity we'll only deal with constant strings for now
  do {
    $token = array_pop($tokenStack);
  } while ($token[0] === T_WHITESPACE);
  if ($token[0] !== T_CONSTANT_ENCAPSED_STRING) { return array('hook'=>'ERROR: could not find hook name'); }
  $hookName = $token[1];

  // look ahead: either we should find a comma (and a second argument) or a close parenthesis
  do {
    $token = array_pop($tokenStack);
  } while ($token[0] === T_WHITESPACE);
  if ($token[0] === T_CLOSE_PARENTHESIS) { return array('hook'=>$hookName); }
  elseif ($token[0] !== T_COMMA) { return array('hook'=>'ERROR: missing a comma or a close parenthesis'); }

  // the next argument should be the $variables array
  $variablesArray = _twig_array_tokens_to_array($tokenStack);

  // check for the closing parenthesis
  do {
    $token = array_pop($tokenStack);
  } while ($token[0] === T_WHITESPACE);
  if ($token[0] !== T_CLOSE_PARENTHESIS) { return array('hook'=>'ERROR: could not find close parenthesis'); }

  // success: return what we've found
  return array('hook'=>$hookName, 'variables'=>$variablesArray);
}


/**
 * Looks for an array in the token stream and build up a PHP array from it.
 */
function _twig_array_tokens_to_array(&$tokenStack) {
  // look for T_ARRAY to start
  do {
    $token = array_pop($tokenStack);
  } while ($token[0] === T_WHITESPACE);
  if ($token[0] !== T_ARRAY) { return array('ERROR: could not find T_ARRAY'); }
  // followed by an open parenthesis
  do {
    $token = array_pop($tokenStack);
  } while ($token[0] === T_WHITESPACE);
  if ($token[0] !== T_OPEN_PARENTHESIS) { return array('ERROR: could not find array\'s open parenthesis'); }

  // run through looking for key=>value pairs
  $result = array();
  do {

    // constant string for the key
    do {
      $token = array_pop($tokenStack);
    } while ($token[0] === T_WHITESPACE);
    if ($token[0] !== T_CONSTANT_ENCAPSED_STRING) { return array('ERROR: could not find array key'); }
    $key = $token[1];

    // => , or )
    do {
      $token = array_pop($tokenStack);
    } while ($token[0] === T_WHITESPACE);
    switch ($token[0]) {
      case T_COMMA:
        // key was value and array keeps going
        $result[] = $key;
        continue;
        break;
      case T_CLOSE_PARENTHESIS:
        // key was value and array stops
        $result[] = $key;
        break 2; // switch and while
      case T_DOUBLE_ARROW:
        // do nothing and continue below looking for the value
        break;
      default:
        return array('ERROR: unexpected token in array');
        break;
    }

    // look for the value
    do {
      $token = array_pop($tokenStack);
    } while ($token[0] === T_WHITESPACE);
    if ($token[0] === T_ARRAY) {
      // if the value is an array, then get the value recursively
      array_push($tokenStack,$token);
      $value = _twig_array_tokens_to_array($tokenStack);
      $result[$key] = $value;
      // look for a comma or close parenthesis
      do {
        $token = array_pop($tokenStack);
      } while ($token[0] === T_WHITESPACE);
      if ($token[0] !== T_COMMA && $token[0] !== T_CLOSE_PARENTHESIS) { return array('ERROR: array ended abnormally'); }
    } else {
      // gobble up everything up to a comma or close parenthesis (while watching for balanced parentheses)
      $value = $token[1];
      $openParentheses=0;
      do {
        $token = array_pop($tokenStack);
        if ($token[0] === T_COMMA) { break; }
        elseif ($token[0] === T_OPEN_PARENTHESIS) {
          $value .= $token[1];
          $openParentheses++;
        }
        elseif ($token[0] === T_CLOSE_PARENTHESIS) {
          if ($openParentheses > 0) {
            $value .= $token[1];
            $openParentheses--;
          }
          else { break; }
        }
        else { $value .= $token[1]; }
      } while (TRUE);
      $result[$key] = $value;
    }

  // keep going until the array closes
  } while ($token[0] !== T_CLOSE_PARENTHESIS);

  // return what we found
  return $result;
}


/**** rules for transforming PHP to Twig ****/

/**
 * Given a rule and a block of PHP, checks to see if the block
 * matches the pattern in the rule.  If so, it substitutes any
 * variable globs into the printf format.
 */
function rule_match($rule,$statementTokenStack) {
  filter_whitespace($statementTokenStack);
  $vars = array();
  foreach ($rule['pattern'] as $type => $content) {
    $token = array_pop($statementTokenStack);
    if (!$token) { return FALSE; }
    if ( ! (
      ($token[0] === $type) ||
      ($token[0] === T_OPEN_PARENTHESIS && $type === T_BOOLEAN_EXPR)
    )) { return FALSE; }
    if ($type === T_STRING && $content !== $token[1]) { return FALSE; }
    // take special actions for some token types
    switch ($type) {
      case T_VARIABLE:
        array_push($statementTokenStack,$token);
        $vars[] = gobble_variable($statementTokenStack);
        break;
      case T_BOOLEAN_EXPR:
        array_push($statementTokenStack,$token);
        $vars[] = gobble_boolean_expression($statementTokenStack);
        break;
      case T_COMMENT:
      case T_DOC_COMMENT:
      // case T_ML_COMMENT: is PHP4 only
        // make sure we don't close the Twig comment
        $vars[] = strtr($token[1], array('#}'=>'#hash}brace'));
        break;
      case T_CONSTANT_ENCAPSED_STRING:
        $vars[] = $token[1];
        break;
      case T_OPEN_TAG:
        // only used for comments; we need it at the end of the $vars array to avoid dinging every other rule
        $whitespaces = explode('<?php',$token[1]);
        $leadWS = $whitespaces[1];
    }
  }
  if (count($statementTokenStack)>0) { return FALSE; }
  else {
    if (isset($leadWS)) { $vars[] = $leadWS; }
    $name = vsprintf($rule['format'],$vars);
    // merde! close tags can sometimes slurp up trailing whitespace
    if ($token[0] === T_CLOSE_TAG) {
      $whitespaces = explode('?>',$token[1]);
      $name .= $whitespaces[1];
    }
    return $name;
  }
}

/**
 * Evaluate all our pattern matching rules on a block.  Return
 * the substituted text (if any), and keep track internally of
 * matching braces.
 */
function apply_rules($statementTokenStack) {
  // get $selfContainedRules, $openBraceRules, and $closeBraceRules
  require 'rules.inc';

  // try selfContainedRules first
  foreach ($selfContainedRules as $rule) {
    $printRule = rule_match($rule,$statementTokenStack);
    if ($printRule !== FALSE) { break; } // we found a match
  }
  if ($printRule !== FALSE) { return $printRule; } // we found a match

  // track open braces
  static $openBraceStack = array();

  // try openBraceRules
  foreach ($openBraceRules as $rule) {
    $printRule = rule_match($rule,$statementTokenStack);
    if ($printRule !== FALSE) { break; } // we found a match
  }
  if ($printRule !== FALSE) {
    // we found a match; ifs and for[each]s increase the brace count
    switch (substr($printRule,0,5)) {
      case '{% if':
        array_push($openBraceStack,T_IF);
        break;
      case '{% for':
        array_push($openBraceStack,T_FOR);
        break;
      // default: elseif/else; do nothing
    }
    return $printRule;
  }

  // try closeBraceRules
  foreach ($closeBraceRules as $rule) {
    $printRule = rule_match($rule,$statementTokenStack);
    if ($printRule !== FALSE) { break; } // we found a match
  }
  if ($printRule !== FALSE) {
    // we found a match
    $tokenType = array_pop($openBraceStack);
    return strtr($printRule,
      ($tokenType === T_IF) ?
      array('end'=>'endif') :
      array('end'=>'endfor')
    );
  }

  // no match on any rule
  return FALSE;
}


/*** "public" methods ***/

/**
 * Takes a string of PHP and returns a string of Twig.
 */
function php_string_to_twig_string($phpString) {
  // get a stream of tokens from the PHP
  $tokens = token_get_all($phpString);

  // keep track of the output string
  $result = '';

  // do our thang
  $tokenStack = preprocess_tokens($tokens);
  while ($token = array_pop($tokenStack)) {
    if ($token[0] === T_OPEN_TAG) {
      array_push($tokenStack, $token);
      $phpTokenStack  = pop_php($tokenStack);
      foreach (php_subblocks($phpTokenStack) as $phpBlockStack) {
        $statementMatch = apply_rules($phpBlockStack);
        $result .= ($statementMatch !== FALSE) ? $statementMatch : print_php($phpBlockStack);
      }
    }
    elseif ($token[0] === T_INLINE_HTML) {
      $result .= $token[1];
    }
    else {
      $result .= unexpected_token_message($token);
      continue;
    }
  }

  // return the twigified PHP
  return $result;
}


/**
 * Converts a theme hook function body to a preprocess function.
 *
 * NB: we assume that the result of using reflection to get the function body via file()
 * gives us this function's body, and it alone.  That is, we match both:
 *   $body =~ /^function $oldFunctionName ($variablesName) {/
 *   $body =~ /}\s*$/  <=  where the closing brace is the closing brace for this function
 *     (and not, say the closing brace for a one-line function that shares the same line
 *     as the closing brace for the function of interest; it's unusual fo shizzle, but is
 *     valid PHP)
 * Actually, the second requirement isn't quite necessary; we stop processing on a balanced
 * close brace.  Also note we needn't mind return $foo vs return($foo); the substitution of
 * return with an assignment works correctly regardless of enclosing parentheses.
 *
 * TODO(?):
 * keep an array of const_strings we encounter, and a second array of variablesName references; output a block of suggested moves to template
 * elementary suggestions (fixed) such as ifs, foreaches, and print/echos
 * rules for assignment, and concat assignment?
 * function to process string globs (including interpolation)?
 *
 * @param string $body
 *   Text of the old theme hook function.  "Body" is a bit of a misnomer; we need the
 *   whole function declaration.
 * @param string $hookName
 *   The Drupal hook we're supposed to be tinkering with.
 * @param string $oldThemeName
 *   The name of the (old) source theme for this function.
 * @param string $newThemeName
 *   The name of the new theme we're creating.
 *
 * For example, for bartik_field__taxonomy_term_reference $oldThemeName = 'bartik' and
 * $hookName = 'field__taxonomy_term_reference'.
 *
 * @return array
 *   'function_text' => text of the function declaration (with comment doc blocks) to insert in template.php
 *   'function_name' => name of the new template preprocess hook
 *   'template_name' => file name for the template to hold the result
 *   'template_token' => array key in $variables used to hold output from the preprocessor
 */
function twig_build_preprocess_function($body, $hookName, $oldThemeName, $newThemeName) {
  $oldFunctionName = $oldThemeName . '_' . $hookName;
  $newFunctionName = $newThemeName . '_template_preprocess_' . $hookName;
  $newTemplateName = strtr($hookName,array('_'=>'-')) . '.html.twig';
  $templateToken   = $newFunctionName; // or set as you please

  $body = '<?php ' . $body;
  $bodyTokens = token_get_all($body);
  $tokenStack = preprocess_tokens($bodyTokens,FALSE);

  // grab the open tag
  $openTagToken = array_pop($tokenStack);
  // grab the function keyword
  $functionToken = array_pop($tokenStack);
  if ($functionToken[0] !== T_FUNCTION) { $output .= '/* ' . unexpected_token_message($functionToken) . ' */'; }

  // grab up to the old function's name
  do {
    $token = array_pop($tokenStack);
  } while ($token[0] === T_WHITESPACE);
  if ($token[0] !== T_STRING || $token[1] !== $oldFunctionName) { $output .= '/* ' . unexpected_token_message($token) . ' */'; }

  // grab the old function's arguments; store the name it uses for $variables
  do {
    $token = array_pop($tokenStack);
  } while ($token[0] === T_WHITESPACE);
  if ($token[0] !== T_OPEN_PARENTHESIS) { $output .= '/* ' . unexpected_token_message($token) . ' */'; }
  $arguments = array();
  $stop = FALSE;
  // FIXME: this will get tripped up by (an inappropriate) pass by reference to the old function
  do {
    $arguments[] = gobble_variable($tokenStack);
    $nextToken = array_pop($tokenStack);
    // check: arguments should be separated by commas and delimited by parentheses
    if ($nextToken[0] !== T_COMMA && $nextToken[0] !== T_CLOSE_PARENTHESIS) { $output .= '/* ' . unexpected_token_message($nextToken) . ' */'; }
    $stop = ($nextToken[0] === T_CLOSE_PARENTHESIS) || empty($tokenStack);
  } while (!$stop);
  if (count($tokenStack)==0 || count($arguments)!=1) { $output .= '/* ' . unexpected_token_message($nextToken) . ' */'; }
  $variablesName = $arguments[0];

  $output = 'function ' . $newFunctionName . '(&$' . $variablesName . ')';

  // grab the function body
  // first grab up to the opening brace
  do {
    $token = array_pop($tokenStack);
    $output .= $token[1];
  } while ($token[0] === T_WHITESPACE);
  if ($token[0] !== T_OPEN_BRACE) { $output .= '/* ' . unexpected_token_message($token) . ' */'; }
  // next grab up to the balancing closing brace
  $openBraceCount = 1;
  $drupalRenderCount = 0;  $tCount = 0;  $constantStringCount = 0;  $variablesRefCount = 0;
  $themeCalls = array();
  do {
    $token = array_pop($tokenStack);
    // accumulate the output
    switch ($token[0]) {
      case T_OPEN_BRACE:
        $openBraceCount++;
        $output .= $token[1];
        break;
      case T_CLOSE_BRACE:
        $openBraceCount--;
        $output .= $token[1];
        break;
      case T_RETURN:
        $output .= '$' . $variablesName . "['" . $templateToken . "'] =";
        break;
      default:
        $output .= $token[1];
        break;
    }
    // check for things to move to the associated template
    switch ($token[0]) {
      case T_STRING:
        switch ($token[1]) {
          case 'drupal_render': $drupalRenderCount++; break;
          case 't': $tCount++; break;
          case 'theme': $themeCalls[] = twig_theme_call_parameters($tokenStack); break;
        }
        break;
      case T_CONSTANT_ENCAPSED_STRING:
        $constantStringCount++;
        break;
      case T_VARIABLE:
        if (substr($token[1],1) === $variablesName) { $variablesRefCount++; }
        break;
    }
  } while ($openBraceCount > 0 && !empty($tokenStack));
  if ($openBraceCount > 0 && empty($tokenStack)) { $output .= '/* ' . unexpected_token_message($token) . ' */'; }
  // we ignore everything after the closing brace (including the close tag)

  // add a trailing newline (because we lost all whitespace after the closing brace)
  $output .= "\n";

  // add a doc block header documenting the new preprocess function
  $headerDocBlock = <<<EOHEADER
/**
 * Prepares variables for [description of $hookName] templates.
 *
 * Default template: $newTemplateName.
 *
 * @param array \$$variablesName
 *   An associative array containing:
 *   - $templateToken: the markup outputted by the original $oldFunctionName theme function.
 */

EOHEADER;
// NB: the last newline is ignored by the heredoc PHP syntax

  // add a doc block footer with suggestions for things to move to the template
  $footerDocBlockStart = <<<EOFOOTERSTART
/**
 * The following could probably be moved to $newTemplateName:

EOFOOTERSTART;
// NB: the last newline is ignored by the heredoc PHP syntax
  $footerDocBlockMiddle = '';
  if ($drupalRenderCount > 0) { $footerDocBlockMiddle .= " *   $drupalRenderCount call(s) to drupal_render()\n"; }
  if ($tCount > 0) { $footerDocBlockMiddle .= " *   $tCount call(s) to t(); can also use t as a filter in Twig\n"; }
  if ($variablesRefCount > 0) { $footerDocBlockMiddle .= " *   $variablesRefCount reference(s) to \$$variablesName; NB: just use the array key in the template (without trying to access $variablesName)\n"; }
  $constantStringCount -= $variablesRefCount;
  if ($constantStringCount > 0) { $footerDocBlockMiddle .= " *   approximately $constantStringCount strings of markup\n"; }
  if (count($themeCalls) > 0) {
    $footerDocBlockMiddle .= " *   " . count($themeCalls) . " call(s) to theme() found.  The function call should be removed and changed to simple arrays:\n";
    foreach ($themeCalls as $argumentsArray) {
      $footerDocBlockMiddle .= "array(\n  '#theme' => " . $argumentsArray['hook'] . ",\n";
      if (isset($argumentsArray['variables'])) {
        foreach ($argumentsArray['variables'] as $key => $value) {
          $key = substr($key,0,1) . '#' . substr($key,1);
          $value = (gettype($value)==='array') ? var_export($value,TRUE) : print_r($value,TRUE); // FIXME: hmm; var_export isn't quite right here either
          $footerDocBlockMiddle .= "  $key => $value,\n";
        }
      }
      $footerDocBlockMiddle .= ")\n";
    }
  }
  $footerDocBlockEnd = " */\n";
  $footerDocBlock = $footerDocBlockMiddle ? $footerDocBlockStart . $footerDocBlockMiddle . $footerDocBlockEnd : '';

  return array(
    'function_text' => $headerDocBlock . $output . $footerDocBlock,
    'function_name' => $newFunctionName,
    'template_name' => $newTemplateName,
    'template_token' => $templateToken,
  );
}
