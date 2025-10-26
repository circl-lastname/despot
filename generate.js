#!/usr/bin/env node

const { chdir } = require("node:process")
const { execFileSync } = require("node:child_process");
const { createHash } = require("node:crypto");
const { existsSync, writeFileSync } = require("node:fs");

console.log("Running exiftool...");

chdir(__dirname);
const metadata = JSON.parse(execFileSync("exiftool", [ "-r", "-j", "music" ], { encoding: "utf-8" }));

let output = {};
let processedFiles = 0;
let importedPictures = 0;

for (file of metadata) {
  if (!file.MIMEType.startsWith("audio/")) {
    console.log(`Skipped non audio file ${file.SourceFile}`);
    continue;
  }
  
  const artist = file.Artist ? file.Artist : "Unknown Artist";
  const album = file.Album ? file.Album : file.FileName;
  const title = file.Title ? file.Title : file.FileName;
  
  const picture = file.Picture ? execFileSync("exiftool", [ "-b", "-picture", file.SourceFile ]) : null;
  let pictureHash = "";
  
  if (picture) {
    const hash = createHash("md5");
    hash.update(picture);
    pictureHash = hash.digest("hex");
  }
  
  if (pictureHash !== "") {
    if (!existsSync(`pictures/${pictureHash}`)) {
      writeFileSync(`pictures/${pictureHash}`, picture);
      console.log(`Imported picture ${pictureHash}`);
      importedPictures++;
    }
  }
  
  if (!output[artist]) {
    output[artist] = {};
  }
  
  if (!output[artist][album]) {
    output[artist][album] = [];
  }
  
  const trackNumber = file.TrackNumber ? parseInt(file.TrackNumber)-1 : output[artist][album].length;
  
  output[artist][album][trackNumber] = {
    title: title,
    file: `${file.SourceFile}`,
    picture: pictureHash !== "" ? `pictures/${pictureHash}` : null,
  };
  
  processedFiles++;
}

writeFileSync("info.json", JSON.stringify(output));

console.log(`Processed ${processedFiles} files, imported ${importedPictures} pictures.`);
