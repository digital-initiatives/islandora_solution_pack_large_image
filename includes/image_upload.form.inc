<?php

/**
 * @file
 * Handles the uploading of the image file.
 */

/**
 * Defines a file upload form for uploading the basic image.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 *
 * @return array
 *   The drupal form definition.
 */
function islandora_large_image_image_upload_form(array $form, array &$form_state) {
  $upload_size = min((int) ini_get('post_max_size'), (int) ini_get('upload_max_filesize'));
  $extensions = array('tiff tif jp2');
  $upload_required = variable_get('islandora_require_obj_upload', TRUE);
  return array(
    'file' => array(
      '#title' => t('Image'),
      '#type' => 'managed_file',
      '#required' => $upload_required,
      '#description' => t('Select image to upload.<br/>Files must be less than <strong>@size MB.</strong><br/>Allowed file types: <strong>@ext.</strong>', array('@size' => $upload_size, '@ext' => $extensions[0])),
      '#default_value' => isset($form_state['values']['files']) ? $form_state['values']['files'] : NULL,
      '#upload_location' => 'temporary://',
      '#upload_validators' => array(
        'file_validate_extensions' => $extensions,
        // Assume its specified in MB.
        'file_validate_size' => array($upload_size * 1024 * 1024),
      ),
    ),
  );
}

/**
 * Submit handler, adds uploaded file to ingest object.
 *
 * @param array $form
 *   The drupal form.
 * @param array $form_state
 *   The drupal form state.
 */
function islandora_large_image_image_upload_form_submit(array $form, array &$form_state) {
  $object = islandora_ingest_form_get_object($form_state);
  $file = file_load($form_state['values']['file']);
  $path = drupal_realpath($file->uri);

  // Height width y_resolution x_resolution compression codec colorspace depth.
  $file_image_properties = exec(escapeshellcmd("identify -format \"height=>%h\r\nwidth=>%W\r\ny_resolution=>%y\r\nx_resolution=>%x\r\ncompression=>%C\r\ncodec=>%m\r\ncolorspace=>%r\r\nbitdepth=>%z\" $path"));

  // Create a temp file with the id as the filename using a .txt extension.
  $base_name = str_replace(':', '-', $object->id);
  file_put_contents("temporary://{$base_name}.txt", $file_image_properties);

  if ($form_state['values']['file']) {
    if (empty($object['OBJ'])) {
      $ds = $object->constructDatastream('OBJ', 'M');
      $object->ingestDatastream($ds);
    }
    else {
      $ds = $object['OBJ'];
    }
    $ds->setContentFromFile($path, FALSE);
    $ds->label = $file->filename;
    $ds->mimetype = $file->filemime;
  }
}
