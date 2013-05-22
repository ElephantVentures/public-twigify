<?php
// $Id twigify.drush.inc 4/15/13 9:53 PM by johnvsc | jzavocki@elephantventures.com

/**
 * @file
 *   Example drush command.
 *
 *   To run this *fun* command, execute `sudo drush --include=./examples mmas`
 *   from within your drush directory.
 *
 *   See `drush topic docs-commands` for more information about command authoring.
 *
 *   You can copy this file to any of the following
 *     1. A .drush folder in your HOME folder.
 *     2. Anywhere in a folder tree below an active module on your site.
 *     3. /usr/share/drush/commands (configurable)
 *     4. In an arbitrary folder specified with the --include option.
 *     5. Drupal's sites/all/drush folder.
 */

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * See `drush topic docs-commands` for a list of recognized keys.
 *
 * @return
 *   An associative array describing your command(s).
 */
function twigify_drush_command() {
  $items = array();

  // The 'twigify' command
  $items['twigify'] = array(
    'description' => "Converts a non-Twig theme into a Twig theme.",
    'arguments'   => array(
      'theme' => 'Enter the machine name of the theme if you want to run through twigify',
    ),
    'options'     => array(
      'spreads' => array(
        'description'   => 'Comma delimited list of themes.',
        'example-value' => 'zen, omega',
      ),
    ),
    'examples'    => array(
      'drush twigify zen',
    ),
    'aliases'     => array('tw8'),
    'bootstrap'   => DRUSH_BOOTSTRAP_DRUSH, // No bootstrap at all.
  );
  // Commandfiles may also add topics.  These will appear in
  // the list of topics when `drush topic` is executed.
  // To view this topic, run `drush --include=/full/path/to/examples topic`
  $items['twigify-usage'] = array(
    'description'        => 'Instructions on how to twigify a Theme',
    'hidden'             => TRUE,
    'topic'              => TRUE,
    'bootstrap'          => DRUSH_BOOTSTRAP_DRUSH,
    'callback'           => 'drush_print_file',
    'callback arguments' => array(dirname(__FILE__) . '/twigify-topic.txt'),
  );

  return $items;
}


/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'. This hook is optional. If a command
 * does not implement this hook, the command's description is used instead.
 *
 * This hook is also used to look up help metadata, such as help
 * category title and summary.  See the comments below for a description.
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function twigify_drush_help($section) {
  switch ($section) {
    case 'drush:twigify':
      return dt("Add help information here");
    // The 'title' meta item is used to name a group of
    // commands in `drush help`.  If a title is not defined,
    // the default is "All commands in ___", with the
    // specific name of the commandfile (e.g. twigify).
    // Command files with less than four commands will
    // be placed in the "Other commands" section, _unless_
    // they define a title.  It is therefore preferable
    // to not define a title unless the file defines a lot
    // of commands.
    case 'meta:twigify:title':
      return dt("twigify commands");
    // The 'summary' meta item is displayed in `drush help --filter`,
    // and is used to give a general idea what the commands in this
    // command file do, and what they have in common.
    case 'meta:twigify:summary':
      return dt("Add a twigify summary here.");
  }
}

/**
 * Implementation of drush_hook_COMMAND_validate().
 *
 * The validate command should exit with
 * `return drush_set_error(...)` to stop execution of
 * the command.  In practice, calling drush_set_error
 * OR returning FALSE is sufficient.  See drush.api.php
 * for more details.
 */
function drush_twigify_validate() {
  if (drush_is_windows()) {
    // $name = getenv('USERNAME');
    // TODO: implement check for elevated process using w32api
    // as sudo is not available for Windows
    // http://php.net/manual/en/book.w32api.php
    // http://social.msdn.microsoft.com/Forums/en/clr/thread/0957c58c-b30b-4972-a319-015df11b427d
  }
  else {
    # check to see if passed theme is installed
  }
}

/**
 * Example drush command callback. This is where the action takes place.
 *
 *
 * In this function, all of Drupal's API is (usually) available, including
 * any functions you have added in your own modules/themes.
 *
 * @see drush_invoke()
 * @see drush.api.php
 */
function drush_twigify($theme_name = 'default') {

  drush_print(twigify_logo());
  drush_print('- - - - - - - - -');

  drush_print("Welcome! ");
  drush_print(" ");
  drush_print("Before we begin, let's make sure you have everything that you need: ");
  drush_print(" 1. The name of your new theme");
  drush_print(" 2. The source theme to convert from installed on this Drupal instance.");
  drush_print(" 3. The UNIX root path to where you want the converted theme to be saved to.");
  drush_print(" ");
  if (drush_prompt("Do you have  / know the information above and want to continue? (y/n)") == 'y') {
    drush_print('- - - - - - - - -');
    // set up theme and necessary classes
    $theme_object = twigify_select_theme();
    $twig         = twigify_init_twigify_class($theme_object);
    $fh           = new FileHandler();
    drush_print('###### ' . $twig->getOldThemeName() . ' is the source theme. ######');

    // walk through the steps
    drush_print(" ");
    drush_print("Create a New Theme Name ");
    drush_print(" ");
    twigify_process_new_theme_name($twig);
    drush_print(" ");
    drush_print("Create a New Theme Directory ");
    drush_print(" ");
    twigify_create_new_theme_directory($twig, $fh);
    twigify_copy_theme_to_new_theme_directory($twig, $fh);
    drush_print(" ");
    drush_print("Create a New Theme Info File ");
    drush_print(" ");
    twigify_create_info_file($twig, $fh);
    drush_print(" ");
    drush_print("Process the TPLs");
    drush_print(" ");
    twigify_process_tpls($twig, $fh);
    drush_print(" ");
    drush_print("Create the Theme Functions ");
    drush_print(" ");
    twigify_process_theme_functions($twig, $fh);
    drush_print('** REMEMBER **');
    drush_print('drush cc all and drush cron to see changes in your new theme layer!');
  }
  else {
    drush_print("When you have the info, comeback and try again!");
  }
  // finish up
  drush_print(twigify_EV_logo());
  drush_print('- - - - - - - - -');
}

/**
 * Command argument complete callback. Provides argument
 * values for shell completion.
 *
 * @return
 *  Array of popular fillings.
 */
function twigify_theme_complete() {
  return array('values' => array('default', 'zen'));
}
