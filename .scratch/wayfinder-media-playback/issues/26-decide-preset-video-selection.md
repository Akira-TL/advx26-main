# Replace Preset Video selection with deterministic cloud rendering

Type: grilling
Status: resolved
Blocked by: 24, 55

## Question

How is the visual selected or generated for a Shared Sound?

## Answer

The competition build does not select from a Preset Video pool. Every READY content item receives a newly rendered visual produced by `Sound-Visualization-Kaleidoscope-effect` from its normalized audio.

The user does not choose a video or visual style. The backend derives a deterministic render seed from the immutable `content_id`, stores that seed with the content record, and supplies it to the headless renderer. Retries for the same unpublished content therefore reproduce the same visual behavior instead of generating a different result.

The renderer targets the normalized sound duration directly. Adding multiple visualization themes, user-controlled style selection, and fallback preset-video selection are outside the first competition implementation.
