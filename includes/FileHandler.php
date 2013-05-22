<?php
// $Id FileHandler.php 4/21/13 11:30 PM by johnvsc | jzavocki@elephantventures.com

/**
 * Class FileHandler
 *
 * Simple wrappers for file handling processes
 */

class FileHandler {

  private $file_name; // the filename of the current file
  private $file_path; // the path to the file with the file name appended
  private $file_directory;
  private $mapped_directory_structure;
  private $fr; // id for an open file resource
  private $file_data;
  private $file_ext;

  /**
   * @param $file_path
   *
   * This assumes that a filename is at the end of the filepath
   *
   */
  public function init($file_path) {
    $this->setFilePath($file_path);
    $this->setFilenameFromFilepath();
    $this->setFileExt($this->getExtension());
  }

  /**
   * Makes sure the file buffer is closed
   */
  public function __destruct() {
    if (isset($this->fr) && $this->doesExist($this->getFilePath())) {
      //$this->writeFile();
      //$this->closeFile();
    }
  }

  /**
   * @param bool $double
   *   the "double" param indicates that we need to get the whole extension:
   *       sample_file.tpl.php
   *
   * @return string
   */
  public function getExtension($index = 1) {
    $position = strrpos($this->file_name, '.', $index);
    return substr($this->file_name, $position);
  }

  public function replaceExtension($old_extension, $new_extension) {
    $this->setFileName(str_replace($old_extension, $new_extension,  $this->file_name));
    $this->setFilePath(str_replace($old_extension, $new_extension, $this->file_path));
  }

  public function setFilenameFromFilepath() {
    $this->file_name = basename($this->file_path);
  }

  /* wrappers for typical file operations */

  public function openFile() {
    $this->fr = fopen($this->file_path, 'wb');
  }

  public function extractFileContents() {
    $this->file_data = file_get_contents($this->file_path);
  }

  public function writeFile() {
    if (isset($this->fr) && isset($this->file_data)) {
      fwrite($this->fr, $this->file_data);
    }
  }

  public function closeFile() {
    if (isset($this->fr)) {
      fclose($this->fr);
    }
  }

  public function saveFileData() {
    $this->openFile();
    $this->writeFile();
    $this->closeFile();
  }

  public function deleteFile() {
    $current_file = $this->getFilePath();
    if (is_readable($current_file)) {
      unlink($current_file);
    }
  }

  /** Helper functions  -> not reliant on Class Variables  */

  /**
   * make sure file directory is writable
   *
   * @param $dir
   *
   * @return bool
   */
  public function chmodDirectory($dir) {
    closedir(opendir($dir));
    if (!chmod($dir, 0777)) {
      throw new Exception('Couldn\’t chmod 0777 – $dir');
    }
    return TRUE;
  }


  function writeBuffer() {
    if (!$handle = @fopen($this->file_path, 'w')) {
      throw new Exception("Cannot open file " . $this->file_path);
    }
    if (@fwrite($handle, $this->file_data) === FALSE) {
      throw new Exception("Cannot write to file " . $this->file_path);
    }
    @fclose($handle);
    return TRUE;
  }

  /**
   * Creates a Directory relative to the root of the current site root
   *
   * @param      $directory_path
   * @param null $directory_name
   */
  public function createDirectory($directory_path, $directory_name = NULL) {
    $fullPath = ($directory_name) ? $directory_path . '/' . $directory_name : $directory_path;
    if (!file_exists($fullPath) && !is_dir($fullPath)) {
      if (!mkdir($fullPath)) {
        throw new Exception('Failed to make directory');
      }
      else {
        $this->setFileDirectory($fullPath);
      }
    }
  }

  /**
   * Does this file or directory exist?
   *
   * @param $fullPath
   *
   * @return bool
   */
  public function doesExist($fullPath){
    $result = is_dir($fullPath);
    if(!$result){
      $result = file_exists($fullPath);
    }
    return $result;
  }
  /**
   * Maps the passed directory
   * Returns array of "flat" directory structure
   * segregated by extension
   *
   * @param string $path
   * @param int    $level
   * @param string $parent
   * @param null   $all_files
   *
   * @return null
   */
  function mapDirectoryStructure($path = '.', $level = 0, $parent = '', &$all_files = NULL) {
    $ignore = array('cgi-bin', '.', '..');
    // Directories to ignore when listing output. Many hosts
    // will deny PHP access to the cgi-bin.

    $dh = @opendir($path);
    // Open the directory to the handle $dh

    while (FALSE !== ($file = readdir($dh))) {
      // Loop through the directory

      if (!in_array($file, $ignore)) {
        // Check that this file is not to be ignored
        //$spaces = str_repeat('&nbsp;', ($level * 4));
        // Just to add spacing to the list, to better
        // show the directory tree.
        if (is_dir("$path/$file")) {
          // Its a directory, so we need to keep reading down...
          //echo "<strong>$spaces $file</strong><br />";
          $this->mapDirectoryStructure("$path/$file", ($level + 1), $file . '/', $all_files);
          // Re-call this same function but on a new directory.
          // this is what makes function recursive.
        }
        else {

          //echo $parent . $file . "<br />";
          $all_files['all'][] = $parent . $file;
          // segregate by extension
          $position = strrpos($file, '.', 1);
          $ext      = substr($file, $position + 1);
          // put all file ext in their separate sub array
          $all_files[$ext][] = $parent . $file;
        }
      }
    };
    closedir($dh);
    // Close the directory handle
    $this->setMappedDirectoryStructure($all_files);
  }


  function copyDirectory($src, $dst) {
    $dir = opendir($src);
    @mkdir($dst);
    while (FALSE !== ($file = readdir($dir))) {
      if (($file != '.') && ($file != '..')) {
        if (is_dir($src . '/' . $file)) {
          $this->copyDirectory($src . '/' . $file, $dst . '/' . $file);
        }
        else {
          copy($src . '/' . $file, $dst . '/' . $file);
        }
      }
    }
    closedir($dir);
  }
  /** End helper functions */

  /** Getters and Setters */

  public function setFileDirectory($file_directory) {
    $this->file_directory = $file_directory;
  }

  public function getFileDirectory() {
    return $this->file_directory;
  }

  public function setFError($fError) {
    $this->fError = $fError;
  }

  public function getFError() {
    return $this->fError;
  }

  public function setFileData($file_data) {
    $this->file_data = $file_data;
  }

  public function getFileData() {
    return $this->file_data;
  }

  public function setFileName($file_name) {
    $this->file_name = $file_name;
  }

  public function getFileName() {
    return $this->file_name;
  }

  public function setFilePath($file_path) {
    $this->file_path = $file_path;
  }

  public function getFilePath() {
    return $this->file_path;
  }

  public function setFileExt($file_ext) {
    $this->file_ext = $file_ext;
  }

  public function getFileExt() {
    return $this->file_ext;
  }

  public function setMappedDirectoryStructure($mapped_directory_structure) {
    $this->mapped_directory_structure = $mapped_directory_structure;
  }

  public function getMappedDirectoryStructure() {
    return $this->mapped_directory_structure;
  }

}
