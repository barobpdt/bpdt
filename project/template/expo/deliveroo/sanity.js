import  sanityClient   from "@sanity/client";
import  ImageUrlBuilder  from "@sanity/image-url";

const client = sanityClient ({
    projectId: 'pp98xk7d',
    dataset: 'food-delivery',
    useCdn:true,
    apiVersion:'2025-06-28'
});

const build = ImageUrlBuilder(client);
export const urlFor = (source)=> build.image(source);

export default client;