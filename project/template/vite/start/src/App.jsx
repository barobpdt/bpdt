import BrandBanner from "./components/BrandBanner";
import CourseCategories from "./components/CourseCategories";
import Courses from "./components/Courses";
import Footer from "./components/Footer";
import Header from "./components/Header";
import Hero from "./components/Hero";
import Testimonials from "./components/Testimonials";
import WhyLearnLive from "./components/WhyLearnLive";

export default function App() {
  return (
    <>
      <Header />
      <main>
        <Hero />
        <BrandBanner />
        <CourseCategories />
        <Courses />
        <WhyLearnLive />
        <Testimonials />
      </main>
      <Footer />
    </>
  );
}
