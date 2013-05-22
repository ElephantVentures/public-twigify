<?php
/** $Id Twigify.php 4/21/13 10:21 PM by johnvsc | jzavocki@elephantventures.com
 *
 * This class provides conversion specific functionality for converting Drupal "PHPTemplate"
 * themes into "Twig" themes
 *
 */


require_once 'luthor.inc';

class Twigify {

  private $oldThemeName;
  private $newThemeName;
  private $theme;
  private $themeData;
  private $themeInfo;
  private $tpls;
  private $templates;
  private $oldThemeDirectory;
  private $newThemeDirectory;
  private $fileData;
  private $newDirectoryStructure;

  public function init($themeData) {
    $this->setOldThemeName($themeData->name);
    $this->setThemeData($themeData);
    $this->setOldThemeDirectory($themeData->filepath);
  }

  /**
   * Takes the tpls in the theme data array and
   * registers the filepath to the data array
   */
  public function assembleTPLS() {
    $theme_data = $this->getThemeData();
    if (!empty($theme_data)) {
      foreach ($theme_data as $key => $value) {
        $is_current_theme = (isset($value['path'])) ? substr_count($value['path'], $this->oldThemeName) : NULL;
        $is_template      = (isset($value['template'])) ? TRUE : FALSE;
        if ($is_template && $is_current_theme) {
          //drush_print('  ' . $key . ': ' );
          //drush_print('     ' . $value['template'] . '.tpl.php');
          $tpls[] = $value['path'] . '/' . $value['template'] . '.tpl.php';
        }
      }
      if (!empty($tpls)) {
        $this->setTpls($tpls);
      }
    }
  }

  /**
   * This is a legacy function that is not used
   *
   * This finds the preprocess functions and the process function from $this->themeData
   * We needed a more robust solution to accommodate edgecases
   */
  public function assembleTemplateProcessors() {
    if (!empty($this->themeData)) {
      foreach ($this->themeData as $key => $value) {
        $is_current_theme = substr_count($value['path'], $this->oldThemeName);
        if (isset($value['preprocess functions']) && $is_current_theme) {
          for ($i = 0; $i < count($value['preprocess functions']); $i++) {
            if (substr_count($value['preprocess functions'][$i], $this->oldThemeName)) {
              $templates['preprocess'][] = $value['preprocess functions'][$i];
            }
          }
        }
        if (isset($value['process functions']) && $is_current_theme) {
          for ($i = 0; $i < count($value['process functions']); $i++) {
            if (substr_count($value['process functions'][$i], $this->oldThemeName)) {
              $templates['process'][] = $value['process functions'][$i];
            }
          }
        }
      }
      if (!empty($templates)) {
        $this->setTemplates($templates);
      }
    }
  }


  /**
   * Pinched from himerus's http://drupal.org/project/omega_tools
   *
   * @param $twigify_class
   */
  function assembleNewInfoFile() {
    $new_info_file    = $this->createInfoFile();
    $info_file_string = $this->buildInfoFile($new_info_file);
    return $info_file_string;
  }

  /**
   * Takes the fileData (which was parsed from the org .info file)
   * and converts it into the format of the new info files
   */
  function buildInfoFile($array, $prefix = FALSE) {
    $info = '';

    $array['version'] = 'VERSION';
    unset($array['datestamp']);
    unset($array['project']);

    foreach ($array as $key => $value) {
      if (is_array($value)) {
        $info .= $key . ":\n"; //$this->buildInfoFile($value, (!$prefix ? $key : "{$prefix}[{$key}]"));
        switch ($key) {
          case 'stylesheets':
            foreach ($value as $vkey => $vvalue ) {
              $info .= "  " . $vkey . ":\n";
              foreach ($vvalue as $cvalue) {
                $info .= "   - " . $cvalue . "\n";
              }
            }
            break;
          case 'settings':
          case 'regions':
            foreach ($value as $ckey => $cvalue) {
              $info .= "   " . $ckey . ": '" . $cvalue . "'\n";
            }
            break;
          case 'scripts':
            foreach ($value as $ckey => $cvalue) {
              $info .= "   - " . $cvalue . "\n";
            }
            break;
        }
        //$info .= $this->buildInfoFile($value, (!$prefix ? $key : "{$prefix}[{$key}]"));
      }
      else {
        $info .= $key;
        $info .= ": " . $value . "\n";
      }
    }
    return $info;
  }


  public function updateThemeInfo() {
    $this->setThemeInfo(drupal_parse_info_file($this->getOldThemeDirectory() . '/' . $this->getOldThemeName() . '.info'));
  }

  /**
   * Takes the file contents (in PHP) and converts them to Twig.
   */
  public function convertToTwig() {
    // read in some helper functions we need; and grrrrrrr: PHP functions always have global scope
    // FIXME?  make the "public" methods static methods of some class (and hide the rest)

    // get a twigified version the file contents
    $result = php_string_to_twig_string($this->getFileData());

    // stuff the twigified PHP back into the file
    $this->setFileData($result);
  }

  /**
   * Pinched from himerus's http://drupal.org/project/omega_tools
   *
   * @param $twigify_class
   */
  public function createInfoFile() {
    $theme_info = $this->getThemeInfo();
    if (isset($theme_info)) {
      // copy the array
      $new_info_file = $theme_info;
      // make changes
      $new_info_file['name']        = $this->getNewThemeName();
      $new_info_file['description'] = 'New Drupal 8 Twig Theme created with the help of Twigify.';
      // $new_info_file['base theme']  = $subtheme->base;
      $new_info_file['engine']     = 'twig';
      $new_info_file['type']       = 'theme';      
      $new_info_file['core']       = '8.x';
      $new_info_file['version']    = '8.x';
      $new_info_file['screenshot'] = 'screenshot.png';

      return $new_info_file;
    }
  }

  /**
   * Helper function the segregates the full filepaths of the theme assets
   * into sibling arrays with the extension as the array identifier -> from the
   * theme directory's structure -> so paths are preserved
   *
   * Also, creates an additional array of "twig" files which is essentially any "tpl"
   * found in the theme layer and a label created with the new extension
   */
  public function processThemeExtensions() {

    if (isset($this->newDirectoryStructure)) {
      foreach ($this->newDirectoryStructure as $keys => $values) {
        switch ($keys) {
          case 'php':
            foreach ($values as $key => $value) {
              if (substr_count($value, '.tpl')) {
                // convert it to a twig format and add it as a new sub array
                $this->newDirectoryStructure['tpl'][]  = $value;
                $this->newDirectoryStructure['twig'][] = str_replace('tpl.php', 'html.twig', $value);
                unset($this->newDirectoryStructure[$keys][$key]);
              }
            }
        }
      }
    }
  }

  //  Getters and Setters

  public function setTpls($tpls) {
    $this->tpls = $tpls;
  }

  public function getTpls() {
    return $this->tpls;
  }

  public function setTemplates($templates) {
    $this->templates = $templates;
  }

  public function getTemplates() {
    return $this->templates;
  }

  public function setNewThemeName($newThemeName) {
    $this->newThemeName = $newThemeName;
  }

  public function getNewThemeName() {
    return $this->newThemeName;
  }

  public function setOldThemeName($oldThemeName) {
    $this->oldThemeName = $oldThemeName;
  }

  public function getOldThemeName() {
    return $this->oldThemeName;
  }

  public function setTheme($theme) {
    $this->theme = $theme;
  }

  public function getTheme() {
    return $this->theme;
  }

  public function setThemeData($themeData) {
    $this->themeData = $themeData;
  }

  public function getThemeData() {
    return $this->themeData;
  }

  public function setNewThemeDirectory($newThemeDirectory) {
    $this->newThemeDirectory = $newThemeDirectory;
  }

  public function getNewThemeDirectory() {
    return $this->newThemeDirectory;
  }

  public function setFileData($fileData) {
    $this->fileData = $fileData;
  }

  public function getFileData() {
    return $this->fileData;
  }

  public function setThemeInfo($themeInfo) {
    $this->themeInfo = $themeInfo;
  }

  public function getThemeInfo() {
    return $this->themeInfo;
  }

  public function setOldThemeDirectory($oldThemeDirectory) {
    $this->oldThemeDirectory = $oldThemeDirectory;
  }

  public function getOldThemeDirectory() {
    return $this->oldThemeDirectory;
  }

  public function setNewDirectoryStructure($newDirectoryStructure) {
    $this->newDirectoryStructure = $newDirectoryStructure;
  }

  public function getNewDirectoryStructure() {
    return $this->newDirectoryStructure;
  }

}
