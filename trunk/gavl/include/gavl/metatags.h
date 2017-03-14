/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/

#ifndef GAVL_METATAGS_H_INCLUDED
#define GAVL_METATAGS_H_INCLUDED

/** \defgroup metatags Defined metadata keys
 *  \ingroup metadata
 *
 *  For interoperabiolity reasons, try to use these macros
 *  as metadata keys whenever possible.
 *
 *  Since 1.4.0
 *
 * @{
 */

/** \brief Title of the song/movie... */

#define GAVL_META_TITLE       "Title"

/** \brief Title of the song/movie... */
#define GAVL_META_SEARCH_TITLE "SearchTitle"

#define GAVL_META_ORIGINAL_TITLE "OriginalTitle"

/** \brief Unique ID within this database */

#define GAVL_META_ID          "ID"
#define GAVL_META_PARENT_ID   "ParentID"

/** \brief Performing artist */

#define GAVL_META_ARTIST      "Artist"

/** \brief Artist of the Album */

#define GAVL_META_ALBUMARTIST "Albumartist"

/** \brief Author */

#define GAVL_META_AUTHOR      "Author"

/** \brief Copyright */

#define GAVL_META_COPYRIGHT   "Copyright"

/** \brief Album */

#define GAVL_META_ALBUM       "Album"

/** \brief Genre */

#define GAVL_META_GENRE       "Genre"

/** \brief Tracknumber within the album or (more generically) index within parent container
 *
 *  Integer starting with 1
 */

#define GAVL_META_TRACKNUMBER "Track"

/** \brief Generic date
 *
 *  YYYY-MM-DD [HH:MM:SS]
 */

#define GAVL_META_DATE        "Date"

/** \brief Creation date
 *
 *  YYYY-MM-DD [HH:MM:SS]
 */

#define GAVL_META_DATE_CREATE "CreationDate"     // YYYY-MM-DD [HH:MM:SS]

/** \brief Modification date
 *
 *  YYYY-MM-DD [HH:MM:SS]
 */

#define GAVL_META_DATE_MODIFY "ModificationDate" // YYYY-MM-DD [HH:MM:SS]

#define GAVL_META_MTIME       "mtime" // time_t (from stat()) as long

/** \brief Generic year
 */

#define GAVL_META_YEAR        "Year"        // YYYY

/** \brief Language
 *
 *  Use this for subtitles or audio streams
 */

#define GAVL_META_LANGUAGE    "Language"    // ISO 639-2/B 3 letter code

/** \brief Comment
 */

#define GAVL_META_COMMENT     "Comment"

/** \brief Related URL
 */

#define GAVL_META_RELURL         "RelURL"

/** \brief Referenced URL (means: Redirector)
 */

#define GAVL_META_REFURL      "RefURL"

/** \brief Software
 *
 *  For media files, this is the multiplexer software.
 *  For single stream this is the encoder software
 */

#define GAVL_META_SOFTWARE    "Software"

/** \brief Person, who created the file
 */

#define GAVL_META_CREATOR     "Creator"

/** \brief Format
 *
 *  For media files, this is the container format.
 *  For single stream this is the name of the codec
 */
#define GAVL_META_FORMAT      "Format"

/** \brief Label
 *
 * For streams it's the label (e.g. "Directors comments")
 * to display in the stream menu
 *
 * In global metadata it's the label which should be displayed when
 * this file is played
 */

#define GAVL_META_LABEL       "Label"

/** \brief Bitrate
 *
 *  Bitrate as integer in bits/s. Can also be a
 *  string (like VBR)
 */

#define GAVL_META_BITRATE     "Bitrate"

/** \brief Valid bits per audio sample
 */

#define GAVL_META_AUDIO_BITS  "BitsPerSample"

/** \brief Valid bits per pixel
 */

#define GAVL_META_VIDEO_BPP   "BitsPerPixel"

/** \brief Vendor of the device/software, which created the file
 */

#define GAVL_META_VENDOR      "Vendor"

/** \brief Model name of the device, which created the file
 */

#define GAVL_META_DEVICE      "Device"

/** \brief Name of the station for Radio or TV streams
 */

#define GAVL_META_STATION      "Station"

#define GAVL_META_STATION_URL  "StationURL"



/** \brief Approximate duration
 */

#define GAVL_META_APPROX_DURATION  "ApproxDuration"

/** \brief MimeType associated with an item
 */
#define GAVL_META_MIMETYPE         "MimeType"

/** \brief Location used for opening this resource
 */
#define GAVL_META_URI              "URI"

/** \brief src attribute, can be either an array (for multiple sources) or a dictionary
    with at least GAVL_META_URI set
 */

#define GAVL_META_SRC              "src"

/* Which index is currently opened */
#define GAVL_META_SRCIDX           "srcidx"

/* Total tracks of the logical parent */
#define GAVL_META_TOTAL_TRACKS     "TotalTracks"

/** \brief "1"  is big endian, 0 else
 */

#define GAVL_META_BIG_ENDIAN       "BigEndian"

/** \brief Movie Actor. Can be array for multiple entries
 */
#define GAVL_META_ACTOR            "Actor"

/** \brief Movie Director. Can be array for multiple entries
 */

#define GAVL_META_DIRECTOR         "Director"

/** \brief Production country. Can be array for multiple entries
 */

#define GAVL_META_COUNTRY          "Country"

/** \brief Movie plot.
 */

#define GAVL_META_PLOT             "Plot"

/** \brief Audio language
 */

#define GAVL_META_AUDIO_LANGUAGE   "AudioLanguage"

/** \brief Subtitle language
 */

#define GAVL_META_SUBTITLE_LANGUAGE   "SubtitleLanguage"

/* Images associated with media content */

/** \brief Cover art
 */

#define GAVL_META_COVER_URL      "CoverURL"

/* For embedded covers: Location is NULL and offset and size are given */
#define GAVL_META_COVER_EMBEDDED "CoverEmbedded"
#define GAVL_META_COVER_OFFSET   "CoverOffset"
#define GAVL_META_COVER_SIZE     "CoverSize"

#define GAVL_META_WALLPAPER_URL  "WallpaperURL"
#define GAVL_META_POSTER_URL     "PosterURL"
#define GAVL_META_ICON           "Icon"

#define GAVL_META_CAN_SEEK   "CanSeek"
#define GAVL_META_CAN_PAUSE  "CanPause"

#define GAVL_META_AVG_BITRATE    "AVGBitrate"   // Float, kbps
#define GAVL_META_AVG_FRAMERATE  "AVGFramerate" // Float

/* Purely informational entries for the global metadata or per Location */
#define GAVL_META_AUDIO_CHANNELS     "Channels"
#define GAVL_META_AUDIO_SAMPLERATE   "Samplerate"
#define GAVL_META_AUDIO_BITRATE      "AudioBitrate"
#define GAVL_META_VIDEO_BITRATE      "VideoBitrate"
#define GAVL_META_VIDEO_RESOLUTION   "VideoResolution"
#define GAVL_META_VIDEO_ASPECT_RATIO "VideoAspectRatio" // Human readable e.g. (16:9)

#define GAVL_META_WIDTH  "w"
#define GAVL_META_HEIGHT "h"
#define GAVL_META_X      "x"
#define GAVL_META_Y      "y"

#define GAVL_META_NUM_CHILDREN       "NumChildren" // Number of children for container items

#define GAVL_META_CHILDREN           "children" // Generic name for children of an element, which must be an array

#define GAVL_META_IDX                "idx"         // Index in parent container
#define GAVL_META_TOTAL              "total"       // Total number (maximum idx + 1)
#define GAVL_META_SHOW               "Show"        // TV Show, this item belongs to
#define GAVL_META_SEASON             "Season"      // Season, this episode belongs to
#define GAVL_META_RATING             "Rating"      // Rating (float, 0.0..1.0)
#define GAVL_META_PARENTAL_CONTROL   "ParentalControl" // mpaa or FSK (or whatever) rating

#define GAVL_META_ERROR              "Error"       // Indicate, that an error occurrent during loading of a track
#define GAVL_META_SELECTED           "Selected"    // Track is selected in a GUI


#define GAVL_META_STREAM_PTS_START "PTSStart"
#define GAVL_META_STREAM_PTS_END   "PTSEnd"
#define GAVL_META_STREAM_DURATION  "duration"
#define GAVL_META_STREAM_RAWSIZE   "rawsize"
#define GAVL_META_STREAM_FORMAT    "fmt"
#define GAVL_META_STREAM_TIMESCALE "timescale"

#define GAVL_META_AUDIO_STREAMS    "astreams"
#define GAVL_META_VIDEO_STREAMS    "vstreams"
#define GAVL_META_TEXT_STREAMS     "tstreams"
#define GAVL_META_OVERLAY_STREAMS  "ostreams"
#define GAVL_META_METADATA         "metadata"
#define GAVL_META_EDL              "edl"
#define GAVL_META_TRACKS           "tracks" // Array containing single tracks

/**
 *
 *
 */

#define GAVL_META_MEDIA_CLASS                    "MediaClass"
#define GAVL_META_CHILD_CLASS                    "ChildClass"

/* Value for class */
#define GAVL_META_MEDIA_CLASS_AUDIO_FILE         "item.audio"
#define GAVL_META_MEDIA_CLASS_VIDEO_FILE         "item.video"
#define GAVL_META_MEDIA_CLASS_SONG               "item.audio.song"
#define GAVL_META_MEDIA_CLASS_MOVIE              "item.video.movie"
#define GAVL_META_MEDIA_CLASS_MOVIE_PART         "item.video.movie.part"
#define GAVL_META_MEDIA_CLASS_TV_EPISODE         "item.video.episode"
#define GAVL_META_MEDIA_CLASS_AUDIO_BROADCAST    "item.audio.broadcast"
#define GAVL_META_MEDIA_CLASS_VIDEO_BROADCAST    "item.video.broadcast"
#define GAVL_META_MEDIA_CLASS_IMAGE              "item.image"
#define GAVL_META_MEDIA_CLASS_FILE               "item.file"

/* Container values */
#define GAVL_META_MEDIA_CLASS_CONTAINER          "container"       // Generic
#define GAVL_META_MEDIA_CLASS_MUSICALBUM         "container.musicalbum" 
#define GAVL_META_MEDIA_CLASS_PLAYLIST           "container.playlist"
#define GAVL_META_MEDIA_CLASS_CONTAINER_ACTOR    "container.category.actor" 
#define GAVL_META_MEDIA_CLASS_CONTAINER_DIRECTOR "container.category.director" 
#define GAVL_META_MEDIA_CLASS_CONTAINER_ARTIST   "container.category.artist" 
#define GAVL_META_MEDIA_CLASS_CONTAINER_COUNTRY  "container.category.country" 
#define GAVL_META_MEDIA_CLASS_CONTAINER_GENRE    "container.category.genre" 
#define GAVL_META_MEDIA_CLASS_TV_SEASON          "container.season"
#define GAVL_META_MEDIA_CLASS_TV_SHOW            "container.tvshow"
#define GAVL_META_MEDIA_CLASS_DIRECTORY          "container.directory"
#define GAVL_META_MEDIA_CLASS_MOVIE_MULTIPART    "container.movie.multipart"

/* Root Containers */
#define GAVL_META_MEDIA_CLASS_ROOT               "container.root" 
#define GAVL_META_MEDIA_CLASS_ROOT_MUSICALBUMS   "container.root.musicalbums" 
#define GAVL_META_MEDIA_CLASS_ROOT_SONGS         "container.root.songs" 
#define GAVL_META_MEDIA_CLASS_ROOT_PLAYLISTS     "container.root.playlists" 
#define GAVL_META_MEDIA_CLASS_ROOT_MOVIES        "container.root.movies"
#define GAVL_META_MEDIA_CLASS_ROOT_TV_SHOWS      "container.root.tvshows"
#define GAVL_META_MEDIA_CLASS_ROOT_WEBRADIO      "container.root.webradio"
#define GAVL_META_MEDIA_CLASS_ROOT_DIRECTORIES   "container.root.directories"
#define GAVL_META_MEDIA_CLASS_ROOT_PHOTOS        "container.root.photos"



/**
 * @}
 */

#endif //  GAVL_METATAGS_H_INCLUDED
