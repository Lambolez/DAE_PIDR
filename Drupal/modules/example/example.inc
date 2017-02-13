<?php
// $Id:$

/**
 * @file
 * 
 * @author  Bart Lamiroy <lamiroy@cse.lehigh.edu>
 * 
 * @version 1.0
 *
 * \brief The example module is just a "Hello World" example showing how to create a PhP module under Drupal.
 * 
 * It consists of 3 files:
 *  - example.info (containing the module declaration for Drupal)
 *  - example.module (containing a simple menu callback declaration)
 *  - example.inc (the actual menu callback function)
 * 
 * @section LICENSE
 * 
 *  This file is part of the DAE Platform Project (DAE).
 *
 *  DAE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DAE is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DAE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
  

/**
 * Implementation of callback function.
 */
function example_generation() 
{
  $output = "Hello World";

  return $output;
}
